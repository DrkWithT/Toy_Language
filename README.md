# Toy Project 2
## DerkWithT at `replit.com`

### Summary
This will be a toy language interpreter using the tree walk approach on a generated AST. Also, this is a work in progress project for fun. Not for production use!

### Features of Rubel
 1. Paradigm(s):
    - Imperative
 2. Type System:
    - Variable types are inferred.
    - Mismatched types for an operator causes runtime errors
    - Variables can be mutable or not
    - Booleans are written as `$T` and `$F`!
 3. Ruby-like syntax and uses similar C-like operators:
    - Example 1: logical operators are `&&, ||`.
    - Example 2: comparison operators are `==, !=, <, <=, >, >=`
    - Example 3: math operators are `+, -, *, /`

### Keywords of Rubel
 - `use`: Includes a module into a script. Specifically, the names of other procedures and constants become visible.
 - `let`: Declares a mutable variable.
 - `const`: Declares a _deeply_ immutable variable. Lists will not be mutable either as `const`!
 - `proc`: Declares a procedure.
 - `if`: Executes a block of statements if its conditional is true.
 - `otherwise`: Executes a block of statements if the last `if` failed.
 - `while`: Executes a block of statements as long as the conditional is true.
 - `end`: Marks the end of a block.
 - return: Returns a value from an expression in a procedure.

### Examples of Rubel
 - See `tests` to get a sense of Rubel's syntax. I will add more tests later as I continue Rubel.

### TODO:
 1. Make recursive descent parser.
 2. Edit AST structs if needed.
 3. Make interpreter (context and tree walker).
 4. Test sample scripts!
