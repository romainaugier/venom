class Number():
    
    def __init__(self, value) -> None:
        self._value = value

    def __add__(self, other: "Number") -> "Number":
        return Number(self._value + other._value)

if __name__ == "__main__":
    x = Number(3)
    y = Number(4)

    z = x + y
    
    print(z)