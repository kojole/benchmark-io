use std::process;
use std::thread;
use std::time::Duration;

extern crate benchmark_io_rust as bench;
use bench::*;

fn main() {
    let config = cli::parse_args();
    config.hello();

    println!("Wait 5 seconds for setup.");
    let do_sleep = thread::spawn(|| thread::sleep(Duration::from_secs(5)));

    if let Err(err) = setup(config).and_then(|mut bench| {
        do_sleep.join().unwrap();
        run(&mut bench)?;
        teardown(&mut bench)
    }) {
        eprintln!("Application error: {:?}", err);
        process::exit(1);
    }
}
