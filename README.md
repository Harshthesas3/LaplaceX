# LaplaceX Ultimate

A desktop-based symbolic Laplace Transform engine developed in C++ using the Win32 API. LaplaceX Ultimate provides instant computation of Laplace transforms, step-by-step explanations, formula references, and calculation history through an interactive graphical interface.

## Features

### Symbolic Laplace Transform Computation

Supports direct computation of Laplace transforms for:

* Constants
* Polynomials (`t`, `t²`, `t³`, ...)
* Exponential functions (`exp(at)`)
* Trigonometric functions (`sin(at)`, `cos(at)`)
* Hyperbolic functions (`sinh(at)`, `cosh(at)`)

### Intelligent Function Parsing

* Custom symbolic expression parser
* Automatic function classification
* Input validation with error handling
* Support for coefficient-based expressions

### Formula Library

Built-in reference library containing standard Laplace transform formulas and examples for all supported function categories.

### Mathematical Explanations

For every calculation, LaplaceX:

* Identifies the function category
* Displays the formula used
* Generates a mathematical explanation of the transformation process

### History Management

* Stores calculation history during runtime
* Saves history to text files
* Records timestamps, functions, and computed transforms

### Desktop GUI

Built entirely using the Windows Win32 API:

* Modern dark-themed interface
* Interactive controls
* Formula display panels
* History viewer
* Real-time status updates

---

## Supported Transform Families

| Function Type | Example Input | Output Form |
| ------------- | ------------- | ----------- |
| Constant      | `5`           | `5/s`       |
| Polynomial    | `t^2`         | `2!/s^3`    |
| Exponential   | `exp(2t)`     | `1/(s-2)`   |
| Sine          | `sin(3t)`     | `3/(s²+9)`  |
| Cosine        | `cos(3t)`     | `s/(s²+9)`  |
| Sinh          | `sinh(2t)`    | `2/(s²-4)`  |
| Cosh          | `cosh(2t)`    | `s/(s²-4)`  |

---

## Architecture

LaplaceX is organized into modular components:

### Core Modules

* **FunctionParser** – Parses symbolic input expressions.
* **LaplaceEngine** – Generates transforms, formulas, and explanations.
* **HistoryManager** – Stores and exports calculation history.
* **FormulaLibrary** – Maintains built-in transform references.
* **ThemeManager** – Handles UI styling.
* **MainWindow** – Manages GUI controls and user interaction.

### Technologies Used

* C++
* Win32 API
* Object-Oriented Design
* Symbolic Expression Parsing
* File I/O
* Custom Mathematical Evaluation Logic

---

## Example

Input:

```text
sin(4t)
```

Output:

```text
4/(s² + 16)
```

Formula Used:

```text
L{sin(at)} = a/(s² + a²)
```

---

## Future Enhancements

Planned improvements include:

* Inverse Laplace transforms
* Partial fraction decomposition
* Unit step and impulse functions
* Time-shifting and frequency-shifting properties
* Convolution theorem support
* Graph visualization
* Export to PDF
* Extended symbolic algebra engine

---

## Developers

**T. Harshith Krishna Sastry**
RV College of Engineering
Academic Year: 2025–2026

---

## License

This project was developed for academic and educational purposes. Contributions, improvements, and extensions are welcome.
