#This class will describe the state of a cell in the simulation.

class Celle:
    #constructor method which will contain a  variable called _status to describe the status of each object of the class Celle.
    def __init__(self):
        self._status = "Dead"

    #class method that will change the status to "Dead" every time it is called
    def settDoed(self):
        self._status = "Dead"

    #class method that will change the status to "Alive" every time it is called
    def settLevende(self):
        self._status = "Alive"

    #class method which will return True or False depending on the status of a cell is set to "Alive" or "Dead".
    def erLevende(self):
        status = True
        if self._status != "Alive":
            status = False
            return status
        return status
            

    #class Method that assigns symbols to each cell depending on its status
    def hentStatusTegn(self):
        tegn = '.'
        #if a cell is "Alive", this method will return "O"
        if self.erLevende() == True:
            tegn = 'O'
            return tegn
        #if a cell is "Dead", this method will return "."
        else:
            return tegn
        
