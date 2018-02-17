use std::process;

extern crate benchmark_io_rust as bench;
use bench::*;

fn main() {
    let config = cli::parse_args();
    config.hello();

    if let Err(err) = setup(&config).and_then(|mut bench| {
        run(&config, &mut bench)?;
        teardown(&config, &mut bench)
    }) {
        eprintln!("Application error: {:?}", err);
        process::exit(1);
    }
}
