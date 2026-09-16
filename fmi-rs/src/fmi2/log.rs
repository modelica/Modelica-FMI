use std::{
    io::{IsTerminal, Write},
    path::Path,
    sync::Mutex,
};

use colored::Colorize;

use crate::fmi2::types::fmi2Status;

pub trait Logger {
    fn log_call(&self, status: fmi2Status, message: &str);
    fn log_message(&self, status: fmi2Status, category: &str, message: &str);
}

pub struct DefaultLogger {
    pub stream: Mutex<Box<dyn Write + Send>>,
    pub is_terminal: bool,
}

impl DefaultLogger {
    pub fn new<S>(stream: S) -> Self
    where
        S: Write + IsTerminal + Send + 'static,
    {
        let is_terminal = stream.is_terminal();
        DefaultLogger {
            stream: Mutex::new(Box::new(stream)),
            is_terminal,
        }
    }

    pub fn from_path<P: AsRef<Path>>(path: P) -> Result<Self, std::io::Error> {
        let file = std::fs::File::create(path)?;
        Ok(Self::new(file))
    }
}

impl Default for DefaultLogger {
    fn default() -> Self {
        DefaultLogger::new(std::io::stderr())
    }
}

impl Logger for DefaultLogger {
    fn log_call(&self, _status: fmi2Status, message: &str) {
        let prefix = if self.is_terminal {
            "[FMI]".bright_black()
        } else {
            "[FMI]".normal()
        };

        let mut guard = self.stream.lock().unwrap_or_else(|e| e.into_inner());

        if let Err(e) = writeln!(&mut *guard, "{prefix} {message}") {
            eprintln!("Failed to write log message: {e}");
        }
    }

    fn log_message(&self, status: fmi2Status, category: &str, message: &str) {
        let message = message.trim_end();

        let mut guard = self.stream.lock().unwrap_or_else(|e| e.into_inner());

        if self.is_terminal {
            let prefix = match status {
                fmi2Status::Ok => "[INFO]".bright_blue(),
                fmi2Status::Warning => "[WARNING]".yellow(),
                fmi2Status::Error => "[ERROR]".bright_red(),
                fmi2Status::Discard => "[DISCARD]".bright_red(),
                fmi2Status::Fatal => "[FATAL]".bright_red(),
                fmi2Status::Pending => "[PENDING]".bright_red(),
            };

            if let Err(e) = writeln!(&mut *guard, "{prefix} [{category}] {message}") {
                eprintln!("Failed to write log message: {e}");
            }
        } else {
            let prefix = match status {
                fmi2Status::Ok => "[INFO]",
                fmi2Status::Warning => "[WARNING]",
                fmi2Status::Error => "[ERROR]",
                fmi2Status::Discard => "[DISCARD]",
                fmi2Status::Fatal => "[FATAL]",
                fmi2Status::Pending => "[PENDING]",
            };

            if let Err(e) = writeln!(&mut *guard, "{prefix} [{category}] {message}") {
                eprintln!("Failed to write log message: {e}");
            }
        };
    }
}
