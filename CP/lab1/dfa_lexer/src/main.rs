#![allow(dead_code)]
use clap::Parser;
use regex::Regex;
use serde::Deserialize;
use std::collections::HashMap;
use std::fs::OpenOptions;
use std::io::Write;
use std::path::PathBuf;

#[derive(Debug, Deserialize)]
struct DFA {
    states: Vec<String>,
    start: String,
    accept: Vec<String>,
    trans: Vec<Transition>,
}
#[derive(Debug, Deserialize)]
struct Transition {
    from: String,
    to: String,
    pattern: String,
}

#[derive(clap::Parser)]
struct Args {
    /// dfa文件路径
    #[clap(short, long, default_value = "min_dfa.json")]
    dfa_file: PathBuf,
    /// 代码文件路径
    #[clap(short, long, default_value = "test_code.txt")]
    code_file: PathBuf,
    /// 输出文件路径
    #[clap(short, long, default_value = "output.txt")]
    output_file: PathBuf,
    /// 是否保留 "\n"，这会使得输出格式不太好看
    #[clap(short, long, default_value_t = false)]
    raw_newline: bool,
}

fn match_symbol(sym: &str, ch: char) -> bool {
    let pattern = format!("^{}$", sym);
    Regex::new(&pattern)
        .map(|re| re.is_match(&ch.to_string()))
        .unwrap_or(false)
}

fn main() {
    let args = Args::parse();
    let json_data = std::fs::read_to_string(&args.dfa_file).expect("Failed to read DFA file");
    let code = std::fs::read_to_string(&args.code_file).expect("Failed to read code file");
    let dfa: DFA = serde_json::from_str(&json_data).unwrap();

    // === 构建转移表 ===
    let mut trans: HashMap<(String, String), String> = HashMap::new();
    for t in &dfa.trans {
        trans.insert((t.from.clone(), t.pattern.clone()), t.to.clone());
    }

    let mut pos = 0;

    // 先清空文件（truncate），再以追加模式打开
    let _ = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open(&args.output_file)
        .expect("Failed to clear output file");
    let mut file = OpenOptions::new()
        .create(true)
        .append(true)
        .open(&args.output_file)
        .expect("Failed to open output file for append");
    while pos < code.len() {
        let mut current_state = dfa.start.clone();
        let mut last_accept_state: Option<String> = None;
        let mut last_accept_pos = pos;

        let chars: Vec<char> = code[pos..].chars().collect();

        for (i, &ch) in chars.iter().enumerate() {
            let mut moved = false;
            for (key, dst) in &trans {
                if key.0 == current_state {
                    if match_symbol(&key.1, ch) {
                        current_state = dst.clone();
                        moved = true;
                        if dfa.accept.contains(&current_state) {
                            last_accept_state = Some(current_state.clone());
                            last_accept_pos = pos + i + 1;
                        }
                        break;
                    }
                }
            }
            if !moved {
                break;
            }
        }

        if let Some(state) = last_accept_state {
            let mut token_text = &code[pos..last_accept_pos];
            if token_text == "\n" && !args.raw_newline {
                token_text = "\\n";
            }
            writeln!(file, "AcceptState: {:<15} Lexeme: '{}'", state, token_text)
                .expect("Failed to write to output file");
            pos = last_accept_pos;
        } else {
            writeln!(
                file,
                "Unrecognized char: '{}'",
                code[pos..].chars().next().unwrap()
            )
            .expect("Failed to write to output file");
            eprintln!(
                "Unrecognized char: '{}'",
                code[pos..].chars().next().unwrap()
            );
            pos += 1;
        }
    }
    println!(
        "Lexical analysis completed. Output written to {:?}",
        args.output_file
    );
}
