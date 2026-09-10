//! SWE4211 Lab 2 Part 6 -- button to LED reaction latency.
//!
//! The LED mirrors the state of a button, sampled at a fixed poll period. The
//! poll period is given in milliseconds and may be fractional or zero; zero
//! polls continuously with no sleep at all.
//!
//! Like the other real-time programs in this lab, this one must be run as the
//! super user (or with CAP_SYS_NICE granted to the binary).

use rppal::gpio::{Gpio, InputPin, OutputPin};
use std::io::Write;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::{env, process};

/// prctl option for setting the timer slack. Not re-exported by every libc
/// version, so it is defined locally.
const PR_SET_TIMERSLACK: libc::c_int = 29;

/// SCHED_FIFO priority for the poll loop. The maximum priority of 99 is shared
/// with kernel threads such as migration and watchdog, and monopolizing it can
/// starve them. A priority in the 70 to 90 range gives this loop precedence
/// over every ordinary task while leaving the kernel's own real-time work room
/// to run.
const RT_PRIORITY: libc::c_int = 80;

/// Optionally confine the poll loop to a single CPU. This matters more here
/// than in the blink program: a zero delay poll loop saturates whichever core
/// it lands on, so pinning it keeps that cost predictable.
const PIN_TO_CPU: Option<usize> = None;

/// Reads CLOCK_MONOTONIC as a nanosecond count. A u64 of nanoseconds covers
/// several centuries of uptime, so the arithmetic below cannot wrap in
/// practice.
#[inline]
fn now_nanos() -> u64 {
    let mut ts: libc::timespec = unsafe { std::mem::zeroed() };
    unsafe {
        libc::clock_gettime(libc::CLOCK_MONOTONIC, &mut ts);
    }
    (ts.tv_sec as u64) * 1_000_000_000 + (ts.tv_nsec as u64)
}

/// Sleeps until an absolute point on the monotonic clock, restarting if a
/// signal interrupts the sleep. Sleeping to an absolute deadline rather than
/// for a relative duration means the time spent reading the button and driving
/// the LED comes out of the poll period instead of being added to it.
#[inline]
fn sleep_until_nanos(target: u64) {
    let mut ts: libc::timespec = unsafe { std::mem::zeroed() };
    ts.tv_sec = (target / 1_000_000_000) as libc::time_t;
    ts.tv_nsec = (target % 1_000_000_000) as _;

    loop {
        let rc = unsafe {
            libc::clock_nanosleep(
                libc::CLOCK_MONOTONIC,
                libc::TIMER_ABSTIME,
                &ts,
                std::ptr::null_mut(),
            )
        };

        // clock_nanosleep reports failure through its return value rather than
        // errno.
        if rc != libc::EINTR {
            break;
        }
    }
}

/// Promotes the calling thread to SCHED_FIFO at the configured priority.
fn set_realtime_priority() -> std::io::Result<()> {
    unsafe {
        let mut param: libc::sched_param = std::mem::zeroed();
        param.sched_priority = RT_PRIORITY;

        if libc::sched_setscheduler(0, libc::SCHED_FIFO, &param) != 0 {
            return Err(std::io::Error::last_os_error());
        }
    }
    Ok(())
}

/// Locks the process image into physical memory so a page fault cannot delay a
/// sample or a transition.
fn lock_memory() -> std::io::Result<()> {
    unsafe {
        if libc::mlockall(libc::MCL_CURRENT | libc::MCL_FUTURE) != 0 {
            return Err(std::io::Error::last_os_error());
        }
    }
    Ok(())
}

/// Reduces kernel timer slack to a single nanosecond. Real-time threads
/// already run with no slack, but this keeps short poll periods honest if the
/// SCHED_FIFO promotion is refused.
fn tighten_timer_slack() {
    unsafe {
        libc::prctl(PR_SET_TIMERSLACK, 1 as libc::c_ulong);
    }
}

/// Restricts the calling thread to a single CPU.
fn pin_to_cpu(cpu: usize) -> std::io::Result<()> {
    unsafe {
        let mut set: libc::cpu_set_t = std::mem::zeroed();
        libc::CPU_ZERO(&mut set);
        libc::CPU_SET(cpu, &mut set);

        if libc::sched_setaffinity(0, std::mem::size_of::<libc::cpu_set_t>(), &set) != 0 {
            return Err(std::io::Error::last_os_error());
        }
    }
    Ok(())
}

/// Touches every page of a stack resident buffer so the stack pages the poll
/// loop will use are already present and dirty before sampling begins.
fn prefault_stack() {
    const DEPTH: usize = 64 * 1024;
    let mut scratch = [0u8; DEPTH];

    for offset in (0..DEPTH).step_by(4096) {
        unsafe {
            std::ptr::write_volatile(&mut scratch[offset], 0);
        }
    }
}

/// Converts a delay expressed in milliseconds into whole nanoseconds. Any
/// finite value of zero or greater is accepted, so sub-millisecond poll
/// periods such as 0.25 are valid, as is 0 for continuous polling.
fn parse_delay_nanos(text: &str) -> Result<u64, String> {
    let millis: f64 = text
        .parse()
        .map_err(|_| format!("'{text}' is not a number"))?;

    if !millis.is_finite() || millis < 0.0 {
        return Err(format!("'{text}' is not a delay of zero or greater"));
    }

    let nanos = (millis * 1_000_000.0).round();

    if nanos > u64::MAX as f64 {
        return Err(format!("'{text}' is too large to represent"));
    }

    Ok(nanos as u64)
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 4 {
        eprintln!("Usage: {} <LED_PIN> <BUTTON_PIN> <DELAY_MS>", args[0]);
        eprintln!("  DELAY_MS may be fractional. A delay of 0 polls continuously.");
        process::exit(1);
    }

    let led_pin: u8 = args[1].parse().expect("Invalid LED pin number");
    let button_pin: u8 = args[2].parse().expect("Invalid button pin number");

    let period_ns = match parse_delay_nanos(&args[3]) {
        Ok(value) => value,
        Err(reason) => {
            eprintln!("Invalid delay: {reason}");
            process::exit(1);
        }
    };

    // All allocation, device setup and terminal output happens before the
    // thread goes real time. Once the loop starts, the only system call in the
    // common path is the sleep.
    let gpio = Gpio::new().expect("Failed to access GPIO");
    let mut led: OutputPin = gpio.get(led_pin).expect("Invalid LED pin").into_output();

    // The button is read as active low, so the internal pull-up gives a
    // defined high level when the switch is open. If the board already
    // provides an external pull-up, change this back to into_input().
    let button: InputPin = gpio
        .get(button_pin)
        .expect("Invalid button pin")
        .into_input_pullup();

    // Leave the pin in the state this program last wrote rather than having
    // rppal restore it during drop.
    led.set_reset_on_drop(false);
    led.set_high();

    // Ctrl-C sets a flag that the loop checks, so the LED is driven to a known
    // state on the way out. Calling process::exit from the handler, as the
    // original did, skipped that cleanup entirely.
    let running = Arc::new(AtomicBool::new(true));
    let signal_flag = Arc::clone(&running);

    ctrlc::set_handler(move || {
        signal_flag.store(false, Ordering::Relaxed);
    })
    .expect("Error setting Ctrl-C handler");

    if period_ns == 0 {
        println!(
            "Monitoring button on GPIO {button_pin} to control LED on GPIO {led_pin}, polling continuously..."
        );
    } else {
        println!(
            "Monitoring button on GPIO {} to control LED on GPIO {} with a {} ms poll period...",
            button_pin,
            led_pin,
            period_ns as f64 / 1_000_000.0
        );
    }
    println!("Press Ctrl-C to stop.");
    let _ = std::io::stdout().flush();

    prefault_stack();

    if let Err(error) = lock_memory() {
        eprintln!("Warning: could not lock memory: {error}");
    }

    tighten_timer_slack();

    if let Some(cpu) = PIN_TO_CPU {
        if let Err(error) = pin_to_cpu(cpu) {
            eprintln!("Warning: could not set CPU affinity: {error}");
        }
    }

    if let Err(error) = set_realtime_priority() {
        eprintln!("Failed to set the scheduler: {error}");
        eprintln!("This program must be run as the super user.");
        process::exit(1);
    }

    // Counts the polls that could not be issued on time. A non-zero count at
    // exit means the requested poll period is shorter than the loop can
    // actually sustain.
    let mut overruns: u64 = 0;
    let mut deadline = now_nanos();

    while running.load(Ordering::Relaxed) {
        if button.is_low() {
            // Button pressed, drive the LED on.
            led.set_low();
        } else {
            // Button released, drive the LED off.
            led.set_high();
        }

        // A zero period means no sleep and no system call, so the loop samples
        // as fast as the GPIO reads allow.
        if period_ns == 0 {
            continue;
        }

        deadline += period_ns;

        // If the loop has fallen behind, resynchronize rather than chasing a
        // deadline it can never reach.
        let now = now_nanos();
        if deadline <= now {
            deadline = now + period_ns;
            overruns += 1;
        }

        sleep_until_nanos(deadline);
    }

    led.set_high();

    println!("\nExiting...");
    if overruns > 0 {
        println!("{overruns} poll(s) missed their deadline.");
    }
}

