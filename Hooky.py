import _hooky

def hook(object):
    def wrapper(function):
        _hooky._hook(object, object.__name__, function, function.__name__)
    return wrapper
    
def hook_member(object, member_name, member):
    _hooky._hook_member(object, object.__name__, member, member_name)
    
def unhook(object):
    _hooky._unhook(object, object.__name__)