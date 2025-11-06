#![allow(dead_code)]

use clap::Parser;
use regex::Regex;
use serde::ser::SerializeStruct;
use serde::{Deserialize, Serialize, Serializer};
use serde_json::Value;
use std::cell::RefCell;
use std::collections::{HashMap, VecDeque};
use std::fmt;
use std::fs::OpenOptions;
use std::io::Write;
use std::path::PathBuf;
use std::rc::Rc;

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

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize)]
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
                Token {
                    kind: kind.to_string(),
                    lexeme: lexeme.to_string(),
                }
            }
            None => Token {
                kind: s.to_string(),
                lexeme: "".to_string(),
            },
        }
    }
}

impl Token {
    pub fn to_key(&self) -> Self {
        match self.kind.as_str() {
            "ID" | "NUM_INT" | "NUM_FLOAT" | "REPEAT" => Token::from(self.kind.as_str()),
            _ => self.clone(),
        }
    }
}

#[derive(Debug, Clone)]
pub enum ASTNode {
    Terminal {
        token: Token,
    },
    NonTerminal {
        kind: String,
        children: Vec<Rc<RefCell<ASTNode>>>,
    },
}

impl ASTNode {
    pub fn fmt_with_indent(&self, f: &mut fmt::Formatter<'_>, indent: usize) -> fmt::Result {
        let pad = "  ".repeat(indent);
        match self {
            ASTNode::Terminal { token } => {
                writeln!(f, "{}Terminal({}: '{}')", pad, token.kind, token.lexeme)
            }
            ASTNode::NonTerminal { kind, children } => {
                writeln!(f, "{}NonTerminal({})", pad, kind)?;
                for child in children {
                    child.borrow().fmt_with_indent(f, indent + 1)?;
                }
                Ok(())
            }
        }
    }
}

impl fmt::Display for ASTNode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        self.fmt_with_indent(f, 0)
    }
}

impl Serialize for ASTNode {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        match self {
            ASTNode::Terminal { token } => {
                let mut state = serializer.serialize_struct("Terminal", 2)?;
                state.serialize_field("kind", "Terminal")?;
                state.serialize_field("token", token)?;
                state.end()
            }
            ASTNode::NonTerminal { kind, children } => {
                let mut state = serializer.serialize_struct("NonTerminal", 3)?;
                state.serialize_field("type", "NonTerminal")?;
                state.serialize_field("kind", kind)?;
                state.serialize_field(
                    "children",
                    &children
                        .iter()
                        .map(|child| child.borrow().clone())
                        .collect::<Vec<_>>(),
                )?;
                state.end()
            }
        }
    }
}

#[derive(clap::Parser)]
struct Args {
    #[clap(short, long, default_value = "min_dfa.json")]
    dfa_file: PathBuf,
    #[clap(short, long, default_value = "test_code.txt")]
    code_file: PathBuf,
    #[clap(short, default_value = "LL1table.json")]
    table_file: PathBuf,
    #[clap(long, default_value = "token.txt")]
    token_file: PathBuf,
    #[clap(long, default_value = "ast.json")]
    ast_file: PathBuf,
    #[clap(short, long, default_value_t = false)]
    raw_newline: bool,
}

#[derive(Debug)]
struct Ll1Table {
    terminals: Vec<Token>,
    non_terminals: Vec<Token>,
    table: HashMap<Token, HashMap<Token, Vec<Token>>>,
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
            if !moved {
                break;
            }
        }

        if let Some(state) = last_accept_state {
            let mut lexeme = code[pos..last_accept_pos].to_string();
            if !raw_newline {
                lexeme = lexeme.replace('\n', "\\n");
            }
            tokens.push(Token {
                kind: state,
                lexeme,
            });
            pos = last_accept_pos;
        } else {
            eprintln!(
                "Unrecognized char: '{}'",
                code[pos..].chars().next().unwrap()
            );
            pos += 1;
        }
    }

    tokens
}

// ===== LL1 Parsing =====
fn parse_ll1(tokens: &[Token], table: &Ll1Table) -> ASTNode {
    let mut input: VecDeque<Token> = tokens
        .iter()
        .filter(|t| t.kind != "WS")
        .map(|t| t.clone())
        .collect();
    input.push_back(Token {
        kind: "#".to_string(),
        lexeme: "".to_string(),
    });

    let terminals = &table.terminals;
    let non_terminals = &table.non_terminals;
    let table = &table.table;

    let mut stack: Vec<(Token, Rc<RefCell<ASTNode>>)> = Vec::new();
    stack.push((
        Token::from("#"),
        Rc::new(RefCell::new(ASTNode::Terminal {
            token: Token::from("#"),
        })),
    ));
    let program_node = Rc::new(RefCell::new(ASTNode::NonTerminal {
        kind: "Program".to_string(),
        children: Vec::new(),
    }));
    stack.push((Token::from("Program"), program_node.clone()));

    while !stack.is_empty() {
        let (top, _) = stack.last().unwrap();
        let current_input = input.front().unwrap();

        if top.kind == current_input.kind && top.kind == "#" {
            println!("Parsing completed successfully.");
            break;
        } else if top.kind == current_input.kind && terminals.contains(top) {
            let (_, parent) = stack.pop().unwrap();
            let token = input.pop_front().unwrap();
            let parent_mut = &mut *parent.borrow_mut();
            if let ASTNode::Terminal { token: token_in } = parent_mut {
                token_in.lexeme = token.lexeme;
            }
        } else if non_terminals.contains(top) {
            let row = table
                .get(&top.to_key())
                .expect(format!("No row for non-terminal '{:?}'", top).as_str());
            if let Some(prod) = row.get(&current_input.to_key()) {
                let (_, parent) = stack.pop().unwrap();
                let parent_mut = &mut *parent.borrow_mut();
                if !(prod.len() == 1 && prod[0].kind == "ε") {
                    let mut nodes_in_order = vec![];
                    for symbol in prod {
                        let node = if non_terminals.contains(symbol) {
                            ASTNode::NonTerminal {
                                kind: symbol.kind.clone(),
                                children: vec![],
                            }
                        } else {
                            ASTNode::Terminal {
                                token: Token {
                                    kind: symbol.kind.clone(),
                                    lexeme: "".to_string(),
                                },
                            }
                        };
                        let node = Rc::new(RefCell::new(node));
                        nodes_in_order.push((symbol.clone(), node));
                    }
                    for (sym, node) in nodes_in_order.iter().rev() {
                        stack.push((sym.clone(), node.clone()));
                    }
                    if let ASTNode::NonTerminal { children, .. } = parent_mut {
                        for (_, node) in nodes_in_order {
                            children.push(node);
                        }
                    }
                }
            } else {
                eprintln!(
                    "Error: No production for non-terminal '{}' with input token '{}'",
                    top.kind, current_input.kind
                );
                break;
            }
        } else {
            eprintln!(
                "Error: Mismatched symbol on stack '{:?}' and input token '{:?}'",
                top, current_input
            );
            break;
        }
    }

    let program_node = program_node.borrow().clone();
    program_node
}

fn load_ll1_table(path: &PathBuf) -> Ll1Table {
    let data = std::fs::read_to_string(path).expect("Failed to read LL1 table JSON");
    let value: Value = serde_json::from_str(&data).unwrap();

    let mut terminals: Vec<Token> = value
        .get("terminals")
        .unwrap()
        .as_array()
        .unwrap()
        .iter()
        .map(|t| Token::from(t.as_str().unwrap()))
        .collect();
    terminals.push(Token::from("#"));
    terminals.push(Token::from("ε"));

    let non_terminals = value
        .get("nonterminals")
        .unwrap()
        .as_array()
        .unwrap()
        .iter()
        .map(|nt| Token::from(nt.as_str().unwrap()))
        .collect();

    let tbl = value.get("table").unwrap().as_object().unwrap();
    let mut table: HashMap<Token, HashMap<Token, Vec<Token>>> = HashMap::new();
    for (nt, row) in tbl {
        let mut row_map = HashMap::new();
        let row_obj = row.as_object().unwrap();
        for (term, prod) in row_obj {
            let mut vec_token: Vec<Token> = prod
                .as_array()
                .unwrap()
                .iter()
                .map(|t| {
                    let s = t.as_str().unwrap();
                    Token::from(s)
                })
                .collect();
            if vec_token.is_empty() {
                vec_token.push(Token::from("ε"));
            }
            row_map.insert(Token::from(term.as_str()), vec_token);
        }
        table.insert(Token::from(nt.as_str()), row_map);
    }
    Ll1Table {
        terminals,
        non_terminals,
        table,
    }
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
        .open(&args.token_file)
        .expect("Failed to clear output file");
    let mut file = OpenOptions::new()
        .create(true)
        .append(true)
        .open(&args.token_file)
        .expect("Failed to open output file for append");

    for token in &tokens {
        writeln!(
            file,
            "AcceptState: {:<15} Lexeme: '{}'",
            token.kind, token.lexeme
        )
        .unwrap();
    }

    println!(
        "Lexical analysis completed. Output written to {:?}",
        args.token_file
    );

    let ll1_table = load_ll1_table(&args.table_file);
    let ll1table_file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open("LL1table_loaded.txt")
        .expect("Failed to create LL1 table output file");
    let mut ll1_writer = std::io::BufWriter::new(ll1table_file);
    writeln!(ll1_writer, "{:#?}", ll1_table).unwrap();
    let ast = parse_ll1(&tokens, &ll1_table);
    let ast_json = serde_json::to_string_pretty(&ast).unwrap();
    let  mut ast_file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open(&args.ast_file)
        .expect("Failed to clear AST output file");
    writeln!(ast_file, "{}", ast_json).unwrap();
    println!("AST written to {:?}", args.ast_file);
}
