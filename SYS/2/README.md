# 操作系统实验报告：Shell的实现

## 基本信息

* **姓名**：巩皓锴
* **学号**：09023321
* **日期**：2025年12月23日

## 实验内容

通过实验，让学生了解Shell实现机制。

## 实验目的

实现具有管道、重定向功能的shell，能够执行一些简单的基本命令，如进程执行、列目录等。

## 设计思路

`sh-rs` 是一个用 Rust 实现的简易 Shell，其核心设计思路是模块化和事件驱动。通过将不同的功能（如输入处理、命令执行、历史记录、提示符显示等）分解到不同的模块中，实现了清晰的代码结构和高度的可维护性。

### 模块化设计

- **`main.rs`**: 作为程序的入口，负责初始化各项服务，并启动主事件循环。
- **`input.rs`**: 专门处理用户输入，包括读取、解析和响应键盘事件。
- **`exec.rs`**: 负责命令的执行。它解析输入字符串，区分内建命令（如 `cd`, `exit`）和外部命令，并创建子进程来执行外部命令。
- **`prompt.rs`**: 管理并显示命令行提示符，能够动态展示当前工作目录等信息。
- **`history.rs`**: 实现命令历史的记录与检索功能，方便用户重复执行之前的命令。
- **`shrc.rs`**: 在 Shell 启动时加载配置文件 (`.shrc`)，允许用户自定义启动行为。
- **`token/mod.rs` 与 `token/env.rs`**: 处理命令的词法分析和环境变量展开，是命令解析的关键部分。
- **`interrupt.rs`**: 管理中断信号（如 `Ctrl+C`），确保 Shell 在接收到中断时能够正确响应，而不会直接退出。
- **`output.rs`**: 处理和重定向子进程的输出，确保命令执行结果能够正确显示在终端上。

### 异步与事件驱动

项目采用异步运行时，使得 Shell 在等待用户输入或命令执行时不会阻塞。主循环异步地等待用户输入，然后将输入分发给相应的模块进行处理，这种事件驱动的模式提高了 Shell 的响应性和效率。



## 主要数据结构及其说明

`sh-rs` 的数据结构设计简洁而高效，以下是几个核心的数据结构：

- **`Token`**: 表示一个最小的词法单元

    ```rust
    #[derive(Debug, PartialEq, Clone)]
    pub enum Token {
        Word(String),
        Pipe,
        RedirectIn,
        RedirectOut,
        RedirectAppend,
    }
    ```

- **`Vec<ComandPart>`**: 在命令解析后，命令和其参数被存储在一个向量中。

- **`ComandPart`**: 表示一个执行单元的抽象语法树 (AST) 节点

    ```rust
    #[derive(Debug)]
    pub enum CommandPart {
        Execute {
            name: String,
            args: Vec<String>,
            stdin: ExecutionSource,
            stdout: ExecutionSource,
        },
    }
    ```

- **`AtomicBool`**: 为了原子化的切换输入等待与程序执行状态，使用AtomBool来避免数据一致性问题。

## 源程序

```rust
// main.rs
use crate::interrupt::sigint_handler;
use crate::token::parse_command_chain;
use std::sync::atomic::{AtomicBool, Ordering};

mod exec;
mod history;
mod input;
mod interrupt;
mod output;
mod prompt;
mod shrc;
mod token;

type Result<T> = std::result::Result<T, Box<dyn std::error::Error>>;

static IS_WAITING_FOR_INPUT: AtomicBool = AtomicBool::new(true);

#[tokio::main]
async fn main() -> Result<()> {
    #[cfg(unix)]
    tokio::task::spawn(sigint_handler());

    if let Err(e) = shrc::load_shrc().await {
        println_error!("Error loading ~/.shrc: {}", e);
    }
    if let Err(e) = history::History::load().await {
        println_error!("Error loading history: {}", e);
    }
    loop {
        IS_WAITING_FOR_INPUT.store(true, Ordering::SeqCst);
        let width = prompt::print_prompt();
        match input::read_command(width).await {
            Ok(input) => {
                let trimmed_input = input.trim();
                if trimmed_input.is_empty() {
                    continue;
                }
                IS_WAITING_FOR_INPUT.store(false, Ordering::SeqCst);
                history::History::save(trimmed_input).await?;

                match parse_command_chain(token::tokenize(trimmed_input)) {
                    Ok(command_parts) => {
                        if let Err(e) = exec::execute_command_parts(command_parts).await {
                            println_error!("Execution error: {}", e);
                        }
                    }
                    Err(e) => println_error!("Parse error: {}", e),
                }
            }
            Err(e) => {
                if e.kind() == std::io::ErrorKind::Interrupted {
                    continue;
                } else if e.kind() == std::io::ErrorKind::UnexpectedEof {
                    break;
                }
                println_error!("Error reading input: {}", e);
                break;
            }
        }
    }
    Ok(())
}
```

```rust
// output.rs
use colored::{Color, Colorize};

pub fn print_with_color(message: &str, color: Color) {
    println!("{}", message.color(color));
}

#[macro_export]
macro_rules! print_error {
    ($($arg:tt)*) => {
        crate::output::print_with_color(&format!($($arg)*), colored::Color::Red)
    };
}

#[macro_export]
macro_rules! println_error {
    ($($arg:tt)*) => {
        crate::output::print_with_color(&format!($($arg)*), colored::Color::Red)
    };
}
```

```rust
// prompt.rs
use colored::Colorize;
use std::io::Write;

pub fn get_prompt() -> (String, u16) {
    // 获取当前工作目录
    let current_dir = std::env::current_dir()
        .unwrap_or_else(|_| std::path::PathBuf::from("?"))
        .display()
        .to_string()
        .blue();
    let green_prompt = "sh>".green();
    // 构建提示符字符串
    (
        format!("{} {} ", current_dir, green_prompt),
        (current_dir.len() + 5) as u16,
    )
}

pub fn print_prompt() -> u16 {
    let (prompt, width) = get_prompt();
    print!("{}", prompt);
    std::io::stdout().flush().unwrap();
    width
}
```

```rust
// shrc.rs
use crate::IS_WAITING_FOR_INPUT;
use crate::token::parse_command_chain;
use crate::{Result, exec, println_error};
use std::env::VarError;
use std::sync::atomic::Ordering;
use tokio::fs;

pub async fn load_shrc() -> Result<()> {
    match std::env::var("HOME").or_else(|_| std::env::var("USERPROFILE")) {
        Ok(home) => {
            let shrc_path = format!("{}/.shrc", home);
            match std::fs::read_to_string(&shrc_path) {
                Ok(contents) => {
                    IS_WAITING_FOR_INPUT.store(false, Ordering::SeqCst);
                    for line in contents.lines() {
                        let trimmed_line = line.trim();
                        if trimmed_line.is_empty() || trimmed_line.starts_with('#') {
                            continue;
                        }
                        let tokens = crate::token::tokenize(trimmed_line);
                        match parse_command_chain(tokens) {
                            Ok(command_parts) => {
                                if let Err(e) = exec::execute_command_parts(command_parts).await {
                                    println_error!("Error executing {}: {}", shrc_path, e);
                                }
                            }
                            Err(e) => println_error!("Parse error in {}: {}", shrc_path, e),
                        }
                    }
                }
                Err(_) => {
                    let file = fs::OpenOptions::new()
                        .create(true)
                        .write(true)
                        .open(&shrc_path)
                        .await?;
                    drop(file);
                }
            }
        }
        Err(_) => return Err(Box::new(VarError::NotPresent)),
    }
    Ok(())
}
```

```rust
// interupt.rs
use crate::{IS_WAITING_FOR_INPUT, print_error, prompt};
use std::sync::atomic::Ordering;
use tokio::signal::unix::{SignalKind, signal};

#[cfg(unix)]
pub async fn sigint_handler() {
    let mut sigint = signal(SignalKind::interrupt()).expect("Failed to set up SIGINT handler");
    loop {
        sigint.recv().await;
        if IS_WAITING_FOR_INPUT.load(Ordering::SeqCst) {
            print_error!("\n\r");
            prompt::print_prompt();
        }
    }
}
```

```rust
// input.rs
use crate::history;
use crossterm::{
    ExecutableCommand, cursor,
    event::{self, Event, KeyCode, KeyEvent, KeyModifiers},
    terminal::{self, ClearType},
};
use std::io::{self, Write};

pub async fn read_command(prompt_with: u16) -> io::Result<String> {
    terminal::enable_raw_mode()?;
    let mut stdout = io::stdout();

    let mut buffer = String::new();
    let mut cursor_pos = 0;
    let mut history_index = 0;

    loop {
        stdout.flush()?;

        if let Event::Key(KeyEvent {
            code, modifiers, ..
        }) = event::read()?
        {
            match (code, modifiers) {
                (KeyCode::Char('c'), KeyModifiers::CONTROL) => {
                    terminal::disable_raw_mode()?;
                    println!("^C");
                    return Err(io::Error::new(io::ErrorKind::Interrupted, "Interrupted"));
                }
                (KeyCode::Char('d'), KeyModifiers::CONTROL) => {
                    if buffer.is_empty() {
                        terminal::disable_raw_mode()?;
                        println!("^D");
                        return Err(io::Error::new(io::ErrorKind::UnexpectedEof, "EOF"));
                    }
                }
                (KeyCode::Enter, _) => {
                    // Check for line continuation
                    if buffer.trim_end().ends_with('\\') {
                        // Remove backslash and trailing whitespace
                        let trimmed = buffer.trim_end();
                        let len_without_backslash = trimmed.len() - 1;
                        buffer.truncate(len_without_backslash);
                        cursor_pos = buffer.len();

                        // Print newline and continuation prompt
                        stdout.write_all(b"\n\r> ")?;
                        continue;
                    }

                    terminal::disable_raw_mode()?;
                    println!(); // Move to next line
                    return Ok(buffer);
                }
                (KeyCode::Backspace, _) => {
                    if cursor_pos > 0 {
                        buffer.remove(cursor_pos - 1);
                        cursor_pos -= 1;

                        stdout.execute(cursor::MoveLeft(1))?;
                        stdout.execute(terminal::Clear(ClearType::UntilNewLine))?;
                        if cursor_pos < buffer.len() {
                            print!("{}", &buffer[cursor_pos..]);
                            stdout.execute(cursor::MoveLeft((buffer.len() - cursor_pos) as u16))?;
                        }
                    }
                }
                (KeyCode::Left, _) => {
                    if cursor_pos > 0 {
                        cursor_pos -= 1;
                        stdout.execute(cursor::MoveLeft(1))?;
                    }
                }
                (KeyCode::Right, _) => {
                    if cursor_pos < buffer.len() {
                        cursor_pos += 1;
                        stdout.execute(cursor::MoveRight(1))?;
                    }
                }
                (KeyCode::Up, _) => {
                    if let Some(history) = history::History::get_by_index(history_index).await {
                        stdout.execute(cursor::MoveToColumn(prompt_with))?;
                        stdout.execute(terminal::Clear(ClearType::UntilNewLine))?;

                        buffer = history;
                        cursor_pos = buffer.len();
                        print!("{}", buffer);
                        history_index += 1;
                        continue;
                    }
                }
                (KeyCode::Down, _) => {
                    stdout.execute(cursor::MoveToColumn(prompt_with))?;
                    stdout.execute(terminal::Clear(ClearType::UntilNewLine))?;
                    if history_index > 0 {
                        history_index -= 1;
                        if let Some(history) = history::History::get_by_index(history_index).await {
                            buffer = history;
                        } else {
                            buffer.clear();
                        }
                        cursor_pos = buffer.len();
                        print!("{}", buffer);
                    } else {
                        buffer.clear();
                        cursor_pos = 0;
                    }
                }
                (KeyCode::Char(c), _) => {
                    if cursor_pos == buffer.len() {
                        buffer.push(c);
                        cursor_pos += 1;
                        print!("{}", c);
                    } else {
                        buffer.insert(cursor_pos, c);
                        cursor_pos += 1;
                        print!("{}", &buffer[cursor_pos - 1..]);
                        stdout.execute(cursor::MoveLeft((buffer.len() - cursor_pos) as u16))?;
                    }
                }
                _ => {}
            }
        }
    }
}
```

```rust
// history.rs
use crate::Result;
use lazy_static::lazy_static;
use tokio::io::{AsyncBufReadExt, AsyncWriteExt};
use tokio::sync::Mutex;

lazy_static! {
    static ref HISTORY: Mutex<Vec<String>> = Mutex::new(Vec::new());
}

pub struct History;

impl History {
    pub async fn load() -> Result<()> {
        let mut history = HISTORY.lock().await;
        match std::env::var("HOME").or_else(|_| std::env::var("USERPROFILE")) {
            Ok(home) => {
                let history_path = format!("{}/.sh_history", home);
                if let Ok(file) = tokio::fs::File::open(&history_path).await {
                    let reader = tokio::io::BufReader::new(file);
                    let mut lines = reader.lines();
                    while let Ok(Some(line)) = lines.next_line().await {
                        history.push(line);
                    }
                }
            }
            Err(_) => {}
        }
        Ok(())
    }
    pub async fn save(command: &str) -> Result<()> {
        let mut history = HISTORY.lock().await;
        history.push(command.to_string());
        drop(history);
        match std::env::var("HOME").or_else(|_| std::env::var("USERPROFILE")) {
            Ok(home) => {
                let history_path = format!("{}/.sh_history", home);
                let mut file = tokio::fs::OpenOptions::new()
                    .create(true)
                    .append(true)
                    .open(&history_path)
                    .await?;
                file.write_all(format!("{}\n", command).as_bytes()).await?;
                Ok(())
            }
            Err(e) => Err(Box::new(e)),
        }
    }

    /// 倒序获取历史命令，0 为最新的命令
    pub async fn get_by_index(index: usize) -> Option<String> {
        let history = HISTORY.lock().await;
        if index < history.len() {
            Some(history[history.len() - 1 - index].clone())
        } else {
            None
        }
    }
}
```

```rust
// exec.rs
use crate::token::{CommandPart, ExecutionSource, PipeEndpoint};
use crate::{Result, println_error};
use std::env;
use std::fs::File;
use std::process::Command;

pub(crate) async fn execute_command_parts(parts: Vec<CommandPart>) -> Result<()> {
    if parts.is_empty() {
        return Ok(());
    }

    // 检查内置命令 (只能是第一个命令)
    let CommandPart::Execute { name, args, .. } = &parts[0];
    match name.as_str() {
        "exit" => std::process::exit(0),
        "cd" => {
            let home_path = env::var("HOME").unwrap_or_else(|_| "/".to_string());
            let path = args.get(0).unwrap_or(&home_path);
            let new_dir = std::path::Path::new(path);
            if let Err(e) = env::set_current_dir(new_dir) {
                println_error!("cd error: {}", e);
            }
            return Ok(());
        }
        _ => {}
    }

    let mut previous_stdout_handle: Option<std::process::ChildStdout> = None;

    // 遍历执行命令链
    for part in parts.into_iter() {
        let CommandPart::Execute {
            name,
            args,
            stdin,
            stdout,
        } = part;

        let mut command = Command::new(&name);
        command.args(&args);

        // --- 设置 STDIN ---
        match stdin {
            ExecutionSource::Inherit => {
                command.stdin(std::process::Stdio::inherit());
            }
            ExecutionSource::Pipe(PipeEndpoint::Read) => {
                if let Some(handle) = previous_stdout_handle.take() {
                    command.stdin(handle);
                } else {
                    // 错误情况：管道没有上游，应继承 stdin
                    command.stdin(std::process::Stdio::inherit());
                }
            }
            ExecutionSource::File(path) => {
                let file = File::open(path)?;
                command.stdin(std::process::Stdio::from(file));
            }
            _ => {}
        }

        // --- 设置 STDOUT ---
        let is_piped = match stdout {
            ExecutionSource::Inherit => {
                command.stdout(std::process::Stdio::inherit());
                false
            }
            ExecutionSource::Pipe(PipeEndpoint::Write) => {
                command.stdout(std::process::Stdio::piped());
                true
            }
            ExecutionSource::File(path) => {
                let file = if path.starts_with(">>") {
                    // 追加重定向
                    File::options().append(true).create(true).open(&path[2..])?
                } else {
                    // 覆盖重定向
                    File::create(path)?
                };
                command.stdout(std::process::Stdio::from(file));
                false
            }
            _ => false,
        };

        // --- 执行 ---
        let mut child = command.spawn()?;

        // --- 存储管道句柄 或 等待完成 ---
        if is_piped {
            // 如果是管道输出，保存输出句柄给下一个命令
            previous_stdout_handle = child.stdout.take();
        } else if previous_stdout_handle.is_none() {
            // 如果不是管道输出，且没有未连接的管道 (即是链条的终点或单个命令)
            child.wait()?;
        }
        // 否则，如果是链条终点，但前面还有未等待的命令，我们只等待链条的最后一个
    }

    Ok(())
}
```

```rust
// token.rs
use crate::Result;
mod env;
use env::expand_env_vars;

// 表示一个最小的词法单元
#[derive(Debug, PartialEq, Clone)]
pub enum Token {
    Word(String),
    Pipe,
    RedirectIn,
    RedirectOut,
    RedirectAppend,
}

// 表示一个执行单元的抽象语法树 (AST) 节点
#[derive(Debug)]
pub enum CommandPart {
    Execute {
        name: String,
        args: Vec<String>,
        stdin: ExecutionSource,
        stdout: ExecutionSource,
    },
}

#[derive(Debug, PartialEq)]
pub enum ExecutionSource {
    Inherit,
    Pipe(PipeEndpoint),
    File(String),
}

#[derive(Debug, PartialEq)]
pub enum PipeEndpoint {
    Read,
    Write,
}

pub fn tokenize(input: &str) -> Vec<Token> {
    let mut tokens = Vec::new();
    let mut chars = input.chars().peekable();
    let mut current = String::new();
    let mut in_quotes = false;

    while let Some(c) = chars.next() {
        match c {
            '"' => {
                in_quotes = !in_quotes;
                // 注意：在最终的 Word 中去除引号
                if !in_quotes && !current.is_empty() {
                    tokens.push(Token::Word(current.to_string()));
                    current.clear();
                }
            }
            // 遇到非引号内的空格，作为分隔符
            ' ' if !in_quotes => {
                if !current.is_empty() {
                    tokens.push(Token::Word(current.to_string()));
                    current.clear();
                }
            }
            // 遇到操作符，作为分隔符
            '|' | '<' | '>' if !in_quotes => {
                if !current.is_empty() {
                    tokens.push(Token::Word(current.to_string()));
                    current.clear();
                }

                // 识别多字符操作符
                match c {
                    '|' => tokens.push(Token::Pipe),
                    '<' => tokens.push(Token::RedirectIn),
                    '>' => {
                        if chars.peek() == Some(&'>') {
                            chars.next(); // 消耗第二个 '>'
                            tokens.push(Token::RedirectAppend);
                        } else {
                            tokens.push(Token::RedirectOut);
                        }
                    }
                    _ => unreachable!(),
                }
            }
            _ => {
                current.push(c);
            }
        }
    }

    // 处理循环结束时剩余的 current
    if !current.is_empty() {
        tokens.push(Token::Word(current.to_string()));
    }

    // 最终清理：去除 Word token 周围的引号（如果存在）
    tokens
        .into_iter()
        .map(|token| {
            if let Token::Word(s) = token {
                let trimmed = s.trim_matches('"').to_string();
                Token::Word(expand_env_vars(&trimmed))
            } else {
                token
            }
        })
        .collect()
}
pub fn parse_command_chain(tokens: Vec<Token>) -> Result<Vec<CommandPart>> {
    let mut parts = Vec::new();
    let mut current_command: Vec<String> = Vec::new();
    let mut iter = tokens.into_iter().peekable();

    // 状态机：跟踪下一个操作符需要什么
    let mut pending_stdin = ExecutionSource::Inherit;

    // 初始输出假设是下一个命令的输入（管道），或继承终端
    let mut pending_stdout = ExecutionSource::Inherit;

    while let Some(token) = iter.next() {
        match token {
            Token::Word(word) => {
                current_command.push(word);
            }
            op @ (Token::Pipe | Token::RedirectIn | Token::RedirectOut | Token::RedirectAppend) => {
                // 1. 检查是否有前一个命令需要封装
                if current_command.is_empty() {
                    return Err(
                        format!("Parse error: Command expected before operator {:?}", op).into(),
                    );
                }

                // 2. 检查操作符
                match op {
                    Token::Pipe => {
                        // 结束当前命令：stdout 设置为 Pipe Write
                        pending_stdout = ExecutionSource::Pipe(PipeEndpoint::Write);

                        // 封装当前命令并推入
                        parts.push(CommandPart::Execute {
                            name: current_command[0].clone(),
                            args: current_command.drain(1..).collect(),
                            stdin: pending_stdin,
                            stdout: pending_stdout,
                        });
                        current_command.clear();

                        // 为下一个命令准备：stdin 设置为 Pipe Read
                        pending_stdin = ExecutionSource::Pipe(PipeEndpoint::Read);
                        pending_stdout = ExecutionSource::Inherit; // 重置下一个命令的默认输出
                    }
                    // 重定向操作：从迭代器中获取文件名
                    _ => {
                        if let Some(Token::Word(filename)) = iter.next() {
                            match op {
                                Token::RedirectIn => {
                                    pending_stdin = ExecutionSource::File(filename)
                                }
                                Token::RedirectOut => {
                                    pending_stdout = ExecutionSource::File(filename)
                                }
                                Token::RedirectAppend => {
                                    // 使用一个特殊的命名约定或结构来区分 Append
                                    pending_stdout =
                                        ExecutionSource::File(format!(">>{}", filename));
                                }
                                _ => unreachable!(),
                            }
                        } else {
                            return Err(format!("Parse error: Redirection operator {:?} must be followed by a filename.", op).into());
                        }
                    }
                }
            }
        }
    }

    // 处理最后一个命令
    if !current_command.is_empty() {
        parts.push(CommandPart::Execute {
            name: current_command[0].clone(),
            args: current_command.drain(1..).collect(),
            stdin: pending_stdin,
            stdout: pending_stdout,
        });
    }

    Ok(parts)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_token_redir() {
        let input = "echo 123 >> a.txt ";
        let tokens = tokenize(input);
        let expected_tokens = vec![
            Token::Word("echo".to_string()),
            Token::Word("123".to_string()),
            Token::RedirectAppend,
            Token::Word("a.txt".to_string()),
        ];
        assert_eq!(tokens, expected_tokens);
        let parts = parse_command_chain(tokens).unwrap();
        assert_eq!(parts.len(), 1);
        let CommandPart::Execute {
            name,
            args,
            stdin,
            stdout,
        } = &parts[0];

        assert_eq!(name, "echo");
        assert_eq!(args, &vec!["123".to_string()]);
        assert_eq!(*stdin, ExecutionSource::Inherit);
        assert_eq!(*stdout, ExecutionSource::File(">>a.txt".to_string()));
    }

    #[test]
    fn test_token_pipe() {
        let input = "echo 123 | cat";
        let tokens = tokenize(input);
        let expected_tokens = vec![
            Token::Word("echo".to_string()),
            Token::Word("123".to_string()),
            Token::Pipe,
            Token::Word("cat".to_string()),
        ];
        assert_eq!(tokens, expected_tokens);
        let parts = parse_command_chain(tokens).unwrap();
        assert_eq!(parts.len(), 2);
        let CommandPart::Execute {
            name,
            args,
            stdin,
            stdout,
        } = &parts[0];
        assert_eq!(name, "echo");
        assert_eq!(args, &vec!["123".to_string()]);
        assert_eq!(*stdin, ExecutionSource::Inherit);
        assert_eq!(*stdout, ExecutionSource::Pipe(PipeEndpoint::Write));
        let CommandPart::Execute {
            name,
            args,
            stdin,
            stdout,
        } = &parts[1];
        assert_eq!(name, "cat");
        assert!(args.is_empty());
        assert_eq!(*stdin, ExecutionSource::Pipe(PipeEndpoint::Read));
        assert_eq!(*stdout, ExecutionSource::Inherit);
    }
}
```

```rust
// token/env.rs
use std::env;

pub fn expand_env_vars(input: &str) -> String {
    let mut out = String::with_capacity(input.len());
    let mut chars = input.chars().peekable();

    while let Some(c) = chars.next() {
        match c {
            '~' => {
                if out.is_empty() && let Ok(home) = env::var("HOME").or_else(|_| env::var("USERPROFILE")) {
                    out.push_str(&home);
                } else {
                    out.push('~');
                }
            }
            '\\' => {
                if let Some('$') = chars.peek().copied() {
                    chars.next();
                    out.push('$');
                } else {
                    out.push('\\');
                }
            }
            '$' => {
                // Handle ${VAR} or $VAR
                if let Some('{') = chars.peek().copied() {
                    // ${VAR}
                    chars.next(); // consume '{'
                    let mut name = String::new();
                    while let Some(nc) = chars.next() {
                        if nc == '}' {
                            break;
                        }
                        name.push(nc);
                    }
                    let val = env::var(&name).unwrap_or_default();
                    out.push_str(&val);
                } else if let Some('$') = chars.peek().copied() {
                    // $$ -> PID
                    chars.next(); // consume second '$'
                    let pid = std::process::id();
                    out.push_str(&pid.to_string());
                } else if let Some(digit @ ('0'..='9')) = chars.peek().copied() {
                    chars.next();
                    let mut script_name = String::new();
                    // $0 -> script name
                    if digit == '0' {
                        script_name = env::args().next().unwrap_or_default();
                    }
                    out.push_str(&script_name);
                } else {
                    // $VAR
                    let mut name = String::new();
                    // First char must be [A-Za-z_]
                    if let Some(nc) = chars.peek().copied() {
                        if nc.is_ascii_alphabetic() || nc == '_' {
                            name.push(nc);
                            chars.next();
                            while let Some(nc2) = chars.peek().copied() {
                                if nc2.is_ascii_alphanumeric() || nc2 == '_' {
                                    name.push(nc2);
                                    chars.next();
                                } else {
                                    break;
                                }
                            }
                            let val = env::var(&name).unwrap_or_default();
                            out.push_str(&val);
                        } else {
                            // Not a valid var name, keep '$'
                            out.push('$');
                        }
                    } else {
                        // '$' at end
                        out.push('$');
                    }
                }
            }
            _ => out.push(c),
        }
    }

    out
}

#[cfg(test)]
mod tests {
    use crate::token::{tokenize, Token};

    #[test]
    fn test_env_expand_basic() {
        unsafe {
            std::env::set_var("FOO_TEST", "hello");
        }
        let input = "echo $FOO_TEST";
        let tokens = tokenize(input);
        let expected_tokens = vec![
            Token::Word("echo".to_string()),
            Token::Word("hello".to_string()),
        ];
        assert_eq!(tokens, expected_tokens);
    }

    #[test]
    fn test_env_expand_braced_and_escape() {
        unsafe {
            std::env::set_var("BAR_TEST", "world");
        }
        let input = "echo ${BAR_TEST} \\$BAR_TEST \\$$BAR_TEST";
        let tokens = tokenize(input);
        let expected_tokens = vec![
            Token::Word("echo".to_string()),
            Token::Word("world".to_string()),
            Token::Word("$BAR_TEST".to_string()),
            Token::Word("$world".to_string()),
        ];
        assert_eq!(tokens, expected_tokens);
    }

    #[test]
    fn test_env_expand_home() {
        unsafe {
            std::env::set_var("HOME", "/home/testuser");
        }
        let input = "cd ~";
        let tokens = tokenize(input);
        let expected_tokens = vec![
            Token::Word("cd".to_string()),
            Token::Word("/home/testuser".to_string()),
        ];
        assert_eq!(tokens, expected_tokens);
    }
}
```



## 程序运行结果及分析

### 实验步骤与测试：

1. 重定向，在shell中执行 `echo hello >> test.txt`，`wc -l < test.txt`

2. 管道，在shell中执行 `ps aux | grep root`

3. 历史记录，在shell中按上下箭头

4. prompt，在 `.shrc` 中写入指令，重启shell

5. 环境变量，在shell中执行 `echo $PATH`，`cat ~/.sh_history`

    

### 结果分析：

1. 成功追加写入文件，wc计数得到1，符合预期

2. 输出如下，符合预期

    ```shell
    root         1  0.0  0.0   3120  2304 hvc0     Sl+  15:08   0:00 /init
    root         6  0.0  0.0   3120  1792 hvc0     Sl+  15:08   0:00 plan9 --control-socket 6 --log-level 4 --server-fd 7 --pipe-fd 9 --log-truncate
    root         9  0.0  0.0   3124   768 ?        Ss   15:08   0:00 /init
    root        10  0.0  0.0   3140  1152 ?        S    15:08   0:00 /init
    root      7405  0.0  0.0   3136   768 ?        Ss   15:42   0:00 /init
    root      7406  0.0  0.0   3136  1024 ?        S    15:42   0:00 /init
    root      7428  0.0  0.0   3128   768 ?        Ss   15:42   0:00 /init
    root      7429  0.0  0.0   3144  1152 ?        S    15:42   0:00 /init
    root      7481  0.0  0.0   3128   768 ?        Ss   15:42   0:00 /init
    root      7482  0.0  0.0   3144  1152 ?        S    15:42   0:00 /init
    harkerh+  9676  0.0  0.0   3596  1920 pts/5    S+   15:52   0:00 grep root
    ```

3. 按上下键可以切换历史记录，符合预期

4. 重启shell看到写入的指令被执行，符合预期

5. 得到

    ```shell
    /home/harkerhand/.vscode-server/data/User/globalStorage/github.copilot-chat/debugCommand:/home/harkerhand/.vscode-server/data/User/globalStorage/github.copilot-chat/copilotCli:/home/harkerhand/.vscode-server/bin/994fd12f8d3a5aa16f17d42c041e5809167e845a/bin/remote-cli:/home/harkerhand/.local/share/pnpm:/home/harkerhand/.cargo/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/lib/wsl/lib:/usr/bin/site_perl:/usr/bin/vendor_perl:/usr/bin/core_perl
    ```

    和

    ```shell
    echo hello >> test.txt
    ps aux | grep root > test.txt
    echo hello >> test.txt
    ps aux | grep root
    echo $PATH
    cat ~/.sh_history
    ```

    符合预期。



## 实验体会

### 遇到的问题及解决：

在终端中按下 CTRL+C 会直接退出终端，但期望是打断当前行缓冲并换行。查阅资料后，选择手动接管信号处理，解决问题。

### 收获与建议：

1. 深入理解了 Shell 的工作原理: 通过亲手实现一个 Shell，对命令的解析、执行、输入输出重定向、进程管理等核心概念有了更深入的理解。
2. 模块化设计的重要性: 清晰的模块划分使得代码易于理解、维护和扩展。例如，当需要改进历史记录功能时，只需要修改 `history.rs` 模块，而不会影响到其他部分。

