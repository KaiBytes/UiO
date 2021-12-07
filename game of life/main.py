"""
This file contains the main program that the user will use. it refers to the class Spillebrett to create the board with cells.
The main program will contain a user inface in which the user can define the dimensions of the board.
The user will also be able to decide whether they want to continue or terminate the program with every iteratiom
Users will be able to see how many iterations of the program they went through, and how many cells are alive
"""

from spillebrett import Spillebrett

def main():
    #the variable rad and kolonne will ask the user to specify the dimensions of the board.
    #the answer will be turned into integers by using the function int()
    rad = int(input("How many rows?: "))
    kolonne = int(input("How many columns?: "))

    bruker = Spillebrett(rad, kolonne)  #the variable "bruker" is assigned an object of the class Spillebrett
    bruker.tegnBrett() #function below draws board on the terminal

    meny = True
    
    """
    While loop that will continue to run as long as the user types " " (space)
    if the user types in "q", the variable "meny" will be assigned the value False and the loop will end.
    if the user types in anythong other than "q" or " ", the program will return "ugyldig input."
    """

    while meny == True:
        fortsette = input("Press space to continue or 'q' to terminate the program: ")
        if fortsette == " ":
            
            bruker.oppdatering() #Prepares new generation of the board
            bruker.tegnBrett() #Shows updates board to the user
            print("Generation: {} - Amount of living cells: {}".format(bruker._generasjonsnummer, bruker.finnAntallLevende()))
            
        elif fortsette == "q":
            print("Program will be terminated now!")
            meny = False
        else:
            print("Invalid input. Try again")

main()
