# Heck

Heck is a multi paradigm, statically typed, general purpose, compiled/JITed programming language with a C-style syntax.

Heck is designed to be able to deduce types whenever possible. In most cases, the user should be able to program as if it were a dynamically-typed language.

Types can also be explicitly declared when necessary.

Heck also aims to maintain consistency and simplicity; it doesn't have a weird typing system like JavaScript.

Here is an example of some code written in Heck:

```javascript

let a = 6
let num b = 4 // explicit type declaration

// generic argument types; will be recompiled when different argument types are passed in
function add(a, b) {
  return a + b // deduces return type each time it's compiled
}

// the function "add" is compiled for two string arguments
let c = add("hello", "world")

// the function "add" is compiled for a number and a string
let d = add(1, "hello")

print(d) // will print "1hello"

```

The main philosiphy behind Heck is that every piece of code should have equivalent machine code. Other high level languages like JavaScript can't always be converted into machine code, so programs are either slow or partially optimized. This results in a program that is part bytecode, part machine code, which takes up extra resources, and has historically led to security exploits.

There is nothing wrong with using bytecode by itself, but mixing it with machine code is not ideal. Of course, some implementations of Heck could be compiled into bytecode, but all valid Heck code must be able to be compiled into an executable binary or JITed on the fly with no funny business involved.

Here is some more code to demonstrate how types and functions work in Heck:

```javascript

// generic declaration
function add(a, b) {
  return a + b
}

// type-specific override
function add(num a, num b) -> string {
  return "" + (a + b)
}

let a = add(1, 2)
print(a) // prints a string

// functions can also be called before their declaration. This is because there shouldn't be any duplicate function declarations to begin with:

let b = sub(1, 2)

function sub(a, b) {
  return a - b
}

let c = sub("hello", "world") // will not compile; you cannot subtract strings

// you can't have different return types in the same function body:

function bar(bool a) { // will not compile; this function returns both strings and numbers
  if (a) {
    return "string"
  }
  return 2
}

// you can make use of more complex generic functions with explicit type arguments:
let d = add<num, string>(1, "2")

function add3<A, B>(A a, B b, num c) {
  return a + b + c
}

let e = add3("hello", "world", 1)
print(e) // "helloworld1"

// finally, you can make forward declarations:

function multiply(a, b);

```

Heck also has namespaces and classes; no prototype BS that people will just try to **emulate classes** with. If you *need* to use something prototype-based, just use a hashmap, hash-table, map, or whatever it's called in your favorite language.

```javascript

namespace foo.bar {
  
  class baz {
    
    num a
    
    num b
    
    private function random() // forward declaration
    
    d = ""
    
  private:
    
    num e
    
    // overrides the "private:" so public methods/variables can be associated with any private counterparts
    public function add(num val) {
      addCounter++
    }
    num addCounter = 0
    
  }
  
}

function foo.bar.baz.random() {
  return 4 // chosen by fair dice roll
}

```

# Separating Declarations and Definitions

Many people believe code should document itself, and that should mean you can read all of a class's methods without having to scroll through all of their implementations.

In Heck, this is done through forward declarations. You can forward declare classes and functions, but this is optional; you probably wouldn't want to put a simple getter method in a separate file.

Forward declarations are not necessary to call a function before it's been defined; the language can handle that for you. Forward declarations are solely to make code more readable and to allow functions and classes to be implemented elsewhere.

You can implement forward-declared items in either the same file or a different file if you use the `export` keyword. `export` essentially `import`s the current file into the target file. This functionality circumvents the necessity of makefiles and a linker, making compiling (or interpreting depending on the implementation) way quicker and easier.

Files can be `import`ed into the current file using the `import` keyword, e.g. `import lib`, and the current file can be `export`ed to other files using the `export` keyword, e.g. `export target`.

You can also `import` specific methods, classes, and namespaces from a file like so: `import lib.class`.

# Context/Scopes

Some people believe declaring variables without a special keyword, e.g. `name = input()`, will make things simpler. This is the case for many languages such as Python and PHP, and any advocacy for such practices is **blatantly wrong**.

First of all, it makes things harder for humans to read, which is the opposite of what was intended. Consider the following Python code:

```python

f = 2

def foo():
  f = 6 # creates a new variable
  print(f) # prints 6

  if True:
    f = 5 # modifies local f!?
    print(f) # prints 5
  
  print(f) # prints 5

run()

print(f) # prints 2
```
Output:
```
6
5
5
2
```
As demonstrated by the example above, creating a new variable and modifying a previously "*declared*" variable have the *exact* same syntax. Of course, any Python programmer would be able to tell the difference, and they would likely tell you to use the `global` keyword, but even an experienced Python programmer will run into issues that they wouldn't face in a language with more strict syntax. For example, if a piece of code was copied into some nested loops and if statements, there could be a duplicate variable name in the nested code, which the programmer wouldn't initially notice. This could be hard to debug, and the fact that declarations and assignment look exactly the same certainly wouldn't help in finding the source of the issue.

In Heck, the `let` keyword is required to declare new variables, which means it's easier to find where a variable was initially declared, code can be copied with no issues, and debugging is much easier.

Languages like Python are supposed to make it easier to write self-documenting code, and while they often succeed in this task, the lack of separation between declarations and assignment often makes it impossible.

In the case where you need to specify the use a global variable, Heck supports the `global` keyword, but it uses it much more intuitively than other languages:

```javascript

let a = 6

function foo() {
  let a = 4
  
  // you can use globals alongside locals
  let b = a + global.a
  
  global.a += 5
}

```

# Implementations

This is currently the only implementation of Heck, and while it's not yet finished, there could be a large variety of compilation targets in the future.

This implementation is written in C, but since Heck is a compiled language, bootstrapping could be possible in the near future. This implementation will continue to be written in C in order to create a C api, and because there is no doubt that Heck is turing complete, there is no reason to rewrite the compiler in itself any time soon.

A bytecode implementation would also be suitable, but there is no reason not to JIT everything. An interactive shell would be compatible with JITed code as long as the syntax tree remains in memory.
