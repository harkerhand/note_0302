#![allow(dead_code)]
use std::collections::HashMap;
use serde::Deserialize;
use regex::Regex;

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

fn match_symbol(sym: &str, ch: char) -> bool {
    if sym == "ε" {
        return false;
    }

    let pattern = format!("^{}$", sym);
    Regex::new(&pattern).map(|re| re.is_match(&ch.to_string())).unwrap_or(false)
}

fn main() {
    let json_data = include_str!("/home/harkerhand/classes/note_0302/CP/lab1/min_dfa.json"); 
    let dfa: DFA = serde_json::from_str(json_data).unwrap();

    // === 构建转移表 ===
    let mut trans: HashMap<(String, String), String> = HashMap::new();
    for t in &dfa.trans {
        trans.insert((t.from.clone(), t.pattern.clone()), t.to.clone());
    }


    // === 读取测试输入 ===
    let input = String::from("x = 123 + 45.6 // test\n");

    let mut pos = 0;
    while pos < input.len() {
        let mut current_state = dfa.start.clone();
        let mut last_accept_state: Option<String> = None;
        let mut last_accept_pos = pos;

        let chars: Vec<char> = input[pos..].chars().collect();

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
            let token_text = &input[pos..last_accept_pos];
            println!("AcceptState: {:<10} Lexeme: '{}'", state, token_text);
            pos = last_accept_pos;
        } else {
            println!("Unrecognized char: '{}'", input[pos..].chars().next().unwrap());
            pos += 1;
        }
    }
}
