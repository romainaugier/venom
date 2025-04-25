class Adder():
            
    def add(x: int, y: int) -> int:
        return x + y

def main() -> int:
    a = Adder.add(3, 18)
    b = Adder.add(3.0, 19)

    return a + b

if __name__ == "__main__":
    main()