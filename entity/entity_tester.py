import unittest
from biology import Biology
from entity import Entity
#from biology.biology import Biology
#from biology.biology_constants import *

class TestDefaults(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """
        Initialize the constructor and read the test genome
        """
        print("==================== Brain Testing ===================")
        cls.guy = Biology(debug=True)
        cls.entity = Entity()


    def setUp(self):
        """
        Setup a new class at each test
        """
        print(f"\n==================== {self._testMethodName} ====================\n")
        print(self._testMethodDoc)
        #self.guy = Biology(debug=True)

    def test01(self):
        pass

if __name__ == '__main__':
    unittest.main()
