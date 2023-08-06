# Toy Project 2
## "DerkWithT" at `replit.com` / DrkWithT at GitHub

### Summary
This will be a toy language interpreter using the tree walk approach on a generated AST. Also, this is a work in progress project for educational fun only. Feel free to fork this.

### Features of Rubel
 1. Paradigm(s):
    - Imperative
    - Dynamic Scoping
 2. Type System:
    - NO "undefined" values for null safety.
    - Variable types are inferred.
    - Mismatched types for an operator causes runtime errors
    - Variables can be mutable or not
    - Booleans are written as `$T` and `$F`!
 3. Syntax uses similar C-like operators:
    - Example 1: logical operators are `&&, ||`.
    - Example 2: comparison operators are `==, !=, <, <=, >, >=`
    - Example 3: math operators are `+, -, *, /, +=, -=`
 4. Syntax disallows control flow statements outside of function declarations to discourage messy code.

### Keywords of Rubel
 - `module`: Declares a module (a separate procedure group) by name.
 - `use`: Includes a module into a script. Specifically, the names of other procedures and constants become visible.
 - `let`: Declares a mutable variable.
 - `const`: Declares a _deeply_ immutable variable. Lists will be immutable _for now_!
 - `set`: Tells Rubel that we're reassigning a variable. Fails on `const` valued variables during runtime!
 - `proc`: Declares a procedure.
 - `if`: Executes a block of statements if its conditional is true.
 - `otherwise`: Executes a block of statements if the last `if` failed.
 - `while`: Executes a block of statements as long as the conditional is true.
 - `end`: Marks the end of a block.
 - return: Returns a value from an expression in a procedure.

### Examples of Rubel
 - See `tests` to get a sense of Rubel's syntax. I will add more tests later as I continue Rubel.

### TODO:
 1. ~~Make parser and `vartypes.h` structures.~~
   - Later add logical expressions.
 2. ~~Make interpreter: create scopes as HashTable using bucket lists~~, native function API, and evaluator as visitor pattern on AST.
 3. Test sample scripts! (Fix segfault on other few test scripts!)
 4. Refactor and test code even more?
