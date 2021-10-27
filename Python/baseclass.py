
class CBaseClass:
    def __init__ (self):
        pass


    def GetMethod (self, classInstance, methodName):
        if (classInstance is None):
            return None
        method = getattr (classInstance, methodName, None)
        if (method is None):
            return None
        if callable (method):
            return method
        return None
