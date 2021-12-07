# GAME OF LIFE

Program which simulates the life and death of cells. The model used is based on Conway's game of life. You can read more about the model here:
https://en.wikipedia.org/wiki/Conway's_Game_of_Life

I made this project for a course given at the university of Oslo. The course gives an introduction to programming with python. You can read more about the course here (in Norwegian):
https://www.uio.no/studier/emner/matnat/ifi/IN1000/v21/index.html

## Technologies

---

- Python v3.8


## Goal and requirements

---

The goal of this project is to simulate "Conway's Game of Life" using a two dimensional matrix. The size of the matrix is specified by the user and consists of cells which can be "alive or "dead". The status of the cells are set according to rules which are specified under "Requirements". The program enables the user to observe the changes in the cell population for each generation by printing out the matrix.

### Requirements

With every new generation, cells update their status depending on the status of neighbouring cells. the neighbours are the cells along the diagonal, vertical and horizontal axis of a given cell. The
status of each sell is set according to the following rules:
* alive
    * cell has three or more living neighbours
* dead
    * cell has less than two living neighbouring cells (underpopulation)
    * cell has more than three living neighbours (overpopulation)

## Key learnings

---

- Object oriented programming
- Matrices
- List manipulation



