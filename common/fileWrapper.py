import copy

class EU4ConfigComposite(object):
    blank_chars = [' ', '\t', '\n']

    # Default initializer for new configuration files,
    # and the initializer with arguments for reading configurations from file
    def __init__(self, key=None, material='', depth=-1):
        self.key = key
        self.raw = material
        self.depth = depth
        self.children = []
        
        char_stack = []
        brace_count = 0
        is_child_key_found = False
        is_expression = False
        is_in_comments = False

        for char in list(material):
            if char == '#':
                is_in_comments = True
                continue
            if is_in_comments:
                if char == '\n':
                    is_in_comments = False
                else:
                    continue

            if char == '=' and brace_count == 0:
                child_key = ''.join(char_stack).strip()
                char_stack = []
                is_child_key_found = True
                continue
                
            if is_child_key_found and char not in EU4ConfigComposite.blank_chars and char != '{':
                is_expression = True
            if is_expression and char == '\n':
                child_value = ''.join(char_stack).strip()
                char_stack = []
                self.children.append(EU4ConfigExpression(child_key, child_value, depth + 1))
                is_child_key_found = False
                is_expression = False
                continue
                
            if char == '{':
                brace_count += 1
                if brace_count == 1: # the outermost braces should not be included in the material of the inner scope
                    is_child_key_found = False
                    continue
                    
            if char == '}':
                brace_count -= 1
                if brace_count == 0:
                    innerMaterial = ''.join(char_stack).strip()
                    char_stack = []
                    self.children.append(EU4ConfigScope(child_key, innerMaterial, depth + 1))
                    is_child_key_found = False
                    is_expression = False
                    continue
                    
            char_stack.append(char)

    @staticmethod
    def readFromFile(path):
        with open(path) as infile:
            return EU4ConfigComposite(None, infile.read(), -1)

    def __getitem__(self, key):
        for child in self.children:
            if child.key == key:
                return child
        return None

    def __repr__(self):
        repr_str = ''
        for child in self.children:
            repr_str += repr(child)
        return repr_str
    
    __str__ = __repr__

    def appendScope(self, arg):
        if isinstance(arg, EU4ConfigScope):
            scope = copy.deepcopy(arg)
            scope.depth = self.depth + 1
        else:
            scope = EU4ConfigScope(arg, '', self.depth + 1)
        self.children.append(scope)

    def appendExpression(self, arg):
        if isinstance(arg, EU4ConfigExpression):
            expression = copy.deepcopy(arg)
            expression.depth = self.depth + 1
        else:
            key, value = arg
            expression = EU4ConfigExpression(key, value, self.depth + 1)
        self.children.append(expression)
    
    def appendList(self, key, lst):
        scope = EU4ConfigScope(key, ' '.join(lst), self.depth + 1)
        self.children.append(scope)

    def findall(self, key):
        result = []
        for child in self.children:
            if child.key == key:
                result.append(child)
        return result


class EU4ConfigExpression(EU4ConfigComposite):
    def __init__(self, key, value, depth):
        self.key = key
        self.value = value
        self.depth = depth

    def __repr__(self):
        return '\t' * self.depth + self.key + ' = ' + self.value + '\n'

    __str__ = __repr__


class EU4ConfigScope(EU4ConfigComposite):
    def __init__(self, key, material='', depth=0):
        super().__init__(key, material, depth)

        self.is_list = False
        
        if not self.children:
            self.is_list = True
            self.children = material.strip().split()

    def __getitem__(self, key):
        if self.is_list and isinstance(key, int):
            return self.children[key]
        else:
            return super().__getitem__(key)

    def __repr__(self):
        basic_indent = '\t' * self.depth
        repr_str = basic_indent + self.key + ' = {\n'
        if self.is_list:
            repr_str += basic_indent + '\t'
        for child in self.children:
            repr_str += str(child)
            if self.is_list:
                repr_str += ' '
        if self.is_list:
            repr_str += '\n'
        repr_str += basic_indent + '}\n'
        return repr_str

    __str__ = __repr__

    