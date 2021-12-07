"""
This class is responsible for the creation of a two dimensional board in which each element is an instance of the class Celle.
Each cell can be either alive or dead, and this status may change depending on the status of the surrounding cells.
This class checks which cells will change status and update each cell for every generation.
"""

import random
from celle import Celle

#defining the class Spillebrett. Spillebrett is norwegian for playing board.
class Spillebrett:
   
    def __init__(self, rader, kolonner):
        self._rader = rader #rows
        self._kolonner = kolonner #columns      
        self._rutenett = [] #matrix
        
        #for loop which will create a two dimensional board based on the arguments given for the parameters rader and kolonner.
        for i in range(self._rader):
            self._rutenett.append([])
            for j in range(self._kolonner):

                self._celle = Celle() #creating an object of the class Celle for every iteration.
                self._rutenett[i].append(self._celle) #appending this object to row i of the list self._rutenett

        
        self._generasjonsnummer = 0 #variable which keeps track of all the generations we looped through after every update.
        self._generer()  #calling class method _generer, which will asign a random state to each cell
        
    #defining class method _generer which will assign random dead and alive states to every object of the class Celle on the board
    def _generer(self):
        for rad in self._rutenett:
            for i in rad:
                
                #variable sjanse will be assigned a random integer with every iteration by using the randint(a,b) method
                #integer is always either 0, 1 or 2 to simulate a 1/3 chance that a cell is alive.
                
                sjanse = random.randint(0, 2)
                
                if sjanse == 1:
                    #cell instance set to alive
                    i.settLevende()
                else:
                    #cell instance set to dead
                    i.settDoed()

    #method which will be used to print out the board
    def tegnBrett(self):

        brett = []
        
        """
        #double for loop that will iterate over every element in self._rutenett
        #each element is an object of the class Celle
        #on each object we call the method .hentStatusTegn()
        #this method will return an "O" when the cell is alive
        #this method will return an "." when the cell is dead
        # """
        
        for rad in self._rutenett:
            nyRad = [] #new row
            brett.append(nyRad) #new row appended to instance variable brett
            for i in rad:
                x = i.hentStatusTegn() # cell status pictogram fetched
                nyRad.append(x) # pictogram added to row
        
        for rad in brett: #print out every row in board on sperate line
            print(rad)
        
        for i in range(0, 2):
            print(" ")

    # finding surrounding neighbours of a specific cell. Used in status updating for each new generation
    def finnNabo(self, rad, kolonne):
        
        naboListe = [] #contains all neighbours of specific cell

        for row in range(rad - 1, rad + 2):
            for column in range(kolonne - 1, kolonne + 2):
                
                #valid neighbour conditions

                if  row >= 0 and row < len(self._rutenett) and column >= 0 and column < len(self._rutenett[0]):
                    naboListe.append(self._rutenett[row][column])
                else:
                    naboListe.append(0)
        return naboListe

    # This method checks which cells will change status and update each cell for every generation.
    def oppdatering(self):
        blirLevende = []
        blirDoed = []
        
        for row in range(0, self._rader):
            for element in range(0, self._kolonner):
                
                naboListe = self.finnNabo(row, element) #fetches surrounding neighbours for each element

                #in finnNabo, every nonexistent element in the list, like borders and corners, will be returned as a 0.
                #in order to remove potential elements that aren't an objective, I created a list called Celler, which will contain all the object in the list that is returned from the class function .finnNabo(rad, kolonne)
                celler = []
                for item in naboListe:
                    if item != 0:
                        celler.append(item)

                antallLevende = 0 #used for counting surrounding live cells

                 #if targeted cell is alive, should not be included in the count of surrounding libing cells
                if self._rutenett[row][element].erLevende() == True:
                    antallLevende = -1 

                #for loop counts amount of living cells surrounding targeted cell
                for cell in celler:
                    if cell.erLevende() == True:
                        antallLevende += 1

                #conditions for flipping state of cells
                if antallLevende < 2 or antallLevende > 3:
                    blirDoed.append(self._rutenett[row][element]) #cell appended to the "to be killed" list
                elif antallLevende == 3:
                    blirLevende.append(self._rutenett[row][element]) #cell appended to "to be resurrected" list

        #kills all cells in the "to be killed" list
        for element in blirDoed:
            element.settDoed()
        #ressurects all cells in the "to be resurrected" list
        for element in blirLevende:
            element.settLevende()

        self._generasjonsnummer += 1

    #method which will return the total amount of living cells on the board
    def finnAntallLevende(self):
        alleLevende = 0
        for rad in self._rutenett:
            for i in rad:
                if i.erLevende() == True:
                    alleLevende += 1
        return alleLevende

        
