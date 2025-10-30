#![allow(dead_code)]
use clap::Parser;
use regex::Regex;
use serde::Deserialize;
use serde_json::Value;
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

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Token {
    pub kind: String,
    pub lexeme: String,
}

impl From<&str> for Token {
    fn from(s: &str) -> Self {
        match s.find('(') {
            Some(start) => {
                let kind = &s[..start];
                let end = s.len() - 1;
                let lexeme = &s[start + 1..end];
                Token { kind: kind.to_string(), lexeme: lexeme.to_string() }
            }
            None => Token { kind: s.to_string(), lexeme: "".to_string() },
        }
    }
}

#[derive(Debug)]
pub enum ASTNode {
    Program(Vec<Box<ASTNode>>),
    StmtList(Vec<Box<ASTNode>>),
    Stmt(Box<ASTNode>),
    DeclStmt(String, Box<ASTNode>),
    AssignStmt(String, Box<ASTNode>),
    Block(Vec<Box<ASTNode>>),
    FuncCall(String, Vec<Box<ASTNode>>),
    IfStmt(Box<ASTNode>, Box<ASTNode>, Option<Box<ASTNode>>),
    Repeat(Box<ASTNode>, Box<ASTNode>),
    Expr(String),
}

#[derive(clap::Parser)]
struct Args {
    #[clap(short, long, default_value = "min_dfa.json")]
    dfa_file: PathBuf,
    #[clap(short, long, default_value = "test_code.txt")]
    code_file: PathBuf,
    #[clap(short, long, default_value = "LL1table.json")]
    table_file: PathBuf,
    #[clap(short, long, default_value = "output.txt")]
    output_file: PathBuf,
    #[clap(short, long, default_value_t = false)]
    raw_newline: bool,
}

fn match_symbol(sym: &str, ch: char) -> bool {
    let pattern = format!("^{}$", sym);
    Regex::new(&pattern)
        .map(|re| re.is_match(&ch.to_string()))
        .unwrap_or(false)
}

fn lex_code(dfa: &DFA, code: &str, raw_newline: bool) -> Vec<Token> {
    let mut tokens = Vec::new();
    let mut trans: HashMap<(String, String), String> = HashMap::new();
    for t in &dfa.trans {
        trans.insert((t.from.clone(), t.pattern.clone()), t.to.clone());
    }

    let mut pos = 0;
    while pos < code.len() {
        let mut current_state = dfa.start.clone();
        let mut last_accept_state: Option<String> = None;
        let mut last_accept_pos = pos;
        let chars: Vec<char> = code[pos..].chars().collect();

        for (i, &ch) in chars.iter().enumerate() {
            let mut moved = false;
            for (key, dst) in &trans {
                if key.0 == current_state && match_symbol(&key.1, ch) {
                    current_state = dst.clone();
                    moved = true;
                    if dfa.accept.contains(&current_state) {
                        last_accept_state = Some(current_state.clone());
                        last_accept_pos = pos + i + 1;
                    }
                    break;
                }
            }
            if !moved { break; }
        }

        if let Some(state) = last_accept_state {
            let mut lexeme = code[pos..last_accept_pos].to_string();
            if !raw_newline {
                lexeme = lexeme.replace('\n', "\\n");
            }
            tokens.push(Token { kind: state, lexeme });
            pos = last_accept_pos;
        } else {
            eprintln!("Unrecognized char: '{}'", code[pos..].chars().next().unwrap());
            pos += 1;
        }
    }

    tokens
}

// ===== LL1 Parsing =====
fn parse_ll1(tokens: &[Token], table: &HashMap<Token, HashMap<Token, Vec<Token>>>) -> ASTNode {
    todo!()
}



fn load_ll1_table(path: &PathBuf) -> HashMap<Token, HashMap<Token, Vec<Token>>> {
    let data = std::fs::read_to_string(path).expect("Failed to read LL1 table JSON");
    let value: Value = serde_json::from_str(&data).unwrap();
    let mut table: HashMap<Token, HashMap<Token, Vec<Token>>> = HashMap::new();

    let tbl = value.get("table").unwrap().as_object().unwrap();
    for (nt, row) in tbl {
        let mut row_map = HashMap::new();
        let row_obj = row.as_object().unwrap();
        for (term, prod) in row_obj {
            let vec_token = prod.as_array().unwrap().iter().map(
                |t| {
                    let s = t.as_str().unwrap();
                    Token::from(s)
                }
            ).collect();
            row_map.insert(Token::from(term.as_str()), vec_token);
        }
        table.insert(Token::from(nt.as_str()), row_map);
    }
    table
}

fn main() {
    let args = Args::parse();
    let json_data = std::fs::read_to_string(&args.dfa_file).expect("Failed to read DFA file");
    let code = std::fs::read_to_string(&args.code_file).expect("Failed to read code file");
    let dfa: DFA = serde_json::from_str(&json_data).unwrap();

    let tokens = lex_code(&dfa, &code, args.raw_newline);

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

    for token in &tokens {
        writeln!(file, "AcceptState: {:<15} Lexeme: '{}'", token.kind, token.lexeme).unwrap();
    }

    println!("Lexical analysis completed. Output written to {:?}", args.output_file);

    let ll1_table = load_ll1_table(&args.table_file);
    println!("LL(1) Parsing Table loaded. {:#?}", ll1_table);
    let ast = parse_ll1(&tokens, &ll1_table);
    println!("AST generated: {:#?}", ast);
}
