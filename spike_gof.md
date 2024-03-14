Design Patterns: Elements of Reusable Object-Oriented Software

Gang of Four Authors:
Erich Gamma, Richard Helm, Ralph Johnson, and John Vlissides

Rule of 3 Creator:
Marshall Cline 1991

Boost is a set of free libraries written in 1999 in C++ which add functions that perform various math functions

Creational Design Patterns

Singleton	The singleton pattern restricts the initialization of a class to ensure that only one instance of the class can be created.
Factory	    The factory pattern takes out the responsibility of instantiating a object from the class to a Factory class.
Abs Factory	Allows us to create a Factory for factory classes.
Builder	    Creating an object step by step and a method to finally get the object instance.
Prototype	Creating a new object instance from another similar instance and then modify according to our requirements.

Structural Design Patterns

Adapter	    Provides an interface between two unrelated entities so that they can work together.
Composite	Used when we have to implement a part-whole hierarchy. For example, a diagram made of other pieces such as circle, square, triangle, etc.
Proxy	    Provide a surrogate or placeholder for another object to control access to it.
Flyweight	Caching and reusing object instances, used with immutable objects. For example, string pool.
Facade	    Creating a wrapper interfaces on top of existing interfaces to help client applications.
Bridge	    The bridge design pattern is used to decouple the interfaces from implementation and hiding the implementation details from the client program.
Decorator	The decorator design pattern is used to modify the functionality of an object at runtime.

Behavioral Design Patterns

Template    Method	used to create a template method stub and defer some of the steps of implementation to the subclasses.
Mediator	used to provide a centralized communication medium between different objects in a system.
Chain	    used to achieve loose coupling in software design where a request from the client is passed to a chain of objects to process them.
Observer	useful when you are interested in the state of an object and want to get notified whenever there is any change.
Strategy	Strategy pattern is used when we have multiple algorithm for a specific task and client decides the actual implementation to be used at runtime.
Command	    Command Pattern is used to implement lose coupling in a request-response model.
State	    State design pattern is used when an Object change it’s behavior based on it’s internal state.
Visitor	    Visitor pattern is used when we have to perform an operation on a group of similar kind of Objects.
Interpreter	defines a grammatical representation for a language and provides an interpreter to deal with this grammar.
Iterator	used to provide a standard way to traverse through a group of Objects.
Memento	    The memento design pattern is used when we want to save the state of an object so that we can restore later on.


Top 5 Common Patterns:
1. Factory:
Provides a way to create objects with flexibility.
Defines an interface for creating objects, but the actual creation is left to subclasses.
Different subclasses can create objects of different types using the same method.
SatOrbit's Make() method is a factory method.

Example in C++:
```
#include <iostream>
#include <memory>

enum ProductId {MINE, YOURS};

// defines the interface of objects the factory method creates.
class Product {
public:
  virtual void print() = 0;
  virtual ~Product() = default;
};

// implements the Product interface.
class ConcreteProductMINE: public Product {
public:
  void print() {
    std::cout << "this=" << this << " print MINE\n";
  }
};

// implements the Product interface.
class ConcreteProductYOURS: public Product {
public:
  void print() {
    std::cout << "this=" << this << " print YOURS\n";
  }
};

// declares the factory method, which returns an object of type Product.
class Creator {
public:
  virtual std::unique_ptr<Product> create(ProductId id) {
    if (ProductId::MINE == id) return std::make_unique<ConcreteProductMINE>();
    if (ProductId::YOURS == id) return std::make_unique<ConcreteProductYOURS>();
    // repeat for remaining products...

    return nullptr;
  }
  virtual ~Creator() = default;
};

int main() {
  // The unique_ptr prevent memory leaks.
  std::unique_ptr<Creator> creator = std::make_unique<Creator>();
  std::unique_ptr<Product> product = creator->create(ProductId::MINE);
  product->print();

  product = creator->create(ProductId::YOURS);
  product->print();
}

//The program output is like

//this=0x6e5e90 print MINE
//this=0x6e62c0 print YOURS
```

2. Abstract Factory:
Creates families of related or dependent objects.
Provides an interface for creating families of products (e.g., different types of tables, chairs, and decorations).
The difference between factory and abstract factory is that factory can only create one type, whereas abstract can make multiple types

Example in C++:
```
#include <iostream>

enum Direction {North, South, East, West};

class MapSite {
public:
  virtual void enter() = 0;
  virtual ~MapSite() = default;
};

class Room : public MapSite {
public:
  Room() :roomNumber(0) {}
  Room(int n) :roomNumber(n) {}
  void setSide(Direction d, MapSite* ms) {
    std::cout << "Room::setSide " << d << ' ' << ms << '\n';
  }
  virtual void enter() {}
  Room(const Room&) = delete; // rule of three
  Room& operator=(const Room&) = delete;
private:
  int roomNumber;
};

class Wall : public MapSite {
public:
  Wall() {}
  virtual void enter() {}
};

class Door : public MapSite {
public:
  Door(Room* r1 = nullptr, Room* r2 = nullptr)
    :room1(r1), room2(r2) {}
  virtual void enter() {}
  Door(const Door&) = delete; // rule of three
  Door& operator=(const Door&) = delete;
private:
  Room* room1;
  Room* room2;
};

class Maze {
public:
  void addRoom(Room* r) {
    std::cout << "Maze::addRoom " << r << '\n';
  }
  Room* roomNo(int) const {
    return nullptr;
  }
};

class MazeFactory {
public:
  MazeFactory() = default;
  virtual ~MazeFactory() = default;

  virtual Maze* makeMaze() const {
    return new Maze;
  }
  virtual Wall* makeWall() const {
    return new Wall;
  }
  virtual Room* makeRoom(int n) const {
    return new Room(n);
  }
  virtual Door* makeDoor(Room* r1, Room* r2) const {
    return new Door(r1, r2);
  }
};

// If createMaze is passed an object as a parameter to use to create rooms, walls, and doors, then you can change the classes of rooms, walls, and doors by passing a different parameter. This is an example of the Abstract Factory (99) pattern.

class MazeGame {
public:
  Maze* createMaze(MazeFactory& factory) {
    Maze* aMaze = factory.makeMaze();
    Room* r1 = factory.makeRoom(1);
    Room* r2 = factory.makeRoom(2);
    Door* aDoor = factory.makeDoor(r1, r2);
    aMaze->addRoom(r1);
    aMaze->addRoom(r2);
    r1->setSide(North, factory.makeWall());
    r1->setSide(East, aDoor);
    r1->setSide(South, factory.makeWall());
    r1->setSide(West, factory.makeWall());
    r2->setSide(North, factory.makeWall());
    r2->setSide(East, factory.makeWall());
    r2->setSide(South, factory.makeWall());
    r2->setSide(West, aDoor);
    return aMaze;
  }
};

int main() {
  MazeGame game;
  MazeFactory factory;
  game.createMaze(factory);
}

//The program output is:

//Maze::addRoom 0x1317ed0
//Maze::addRoom 0x1317ef0
//Room::setSide 0 0x1318340
//Room::setSide 2 0x1317f10
//Room::setSide 1 0x1318360
//Room::setSide 3 0x1318380
//Room::setSide 0 0x13183a0
//Room::setSide 2 0x13183c0
//Room::setSide 1 0x13183e0
//Room::setSide 3 0x1317f10
```

3. Adapter Pattern:
Allows incompatible interfaces to work together.
Converts the interface of one class into another interface that clients expect.

Example in C++:
```
// Adaptee (Incompatible Interface)
class Adaptee {
public:
    void specificRequest() {
        std::cout << "Adaptee's specific request." << std::endl;
    }
};

// Target (Expected Interface)
class Target {
public:
    virtual void request() = 0;
};

// Adapter (Converts Adaptee to Target)
class Adapter : public Target {
private:
    Adaptee* adaptee;

public:
    Adapter(Adaptee* a) : adaptee(a) {}
    void request() override {
        adaptee->specificRequest();
    }
};

int main() {
    Adaptee* adaptee = new Adaptee();
    Target* adapter = new Adapter(adaptee);
    adapter->request();
    delete adapter;
    delete adaptee;
    return 0;
}
```

4. Observer Pattern:
Defines a one-to-many dependency between objects.
When one object changes state, all its dependents (observers) are notified and updated automatically.

Example in C++:
```
// Subject (Observable)
class Subject {
private:
    std::vector<class Observer*> observers;

public:
    void attach(Observer* observer) {
        observers.push_back(observer);
    }

    void detach(Observer* observer) {
        // Remove observer from the list
    }

    void notify() {
        for (auto observer : observers) {
            observer->update();
        }
    }
};

// Observer
class Observer {
public:
    virtual void update() = 0;
};

// Concrete Observer
class ConcreteObserver : public Observer {
public:
    void update() override {
        std::cout << "ConcreteObserver received an update." << std::endl;
    }
};

int main() {
    Subject* subject = new Subject();
    Observer* observer = new ConcreteObserver();

    subject->attach(observer);
    // ... Some state change in subject triggers notify()

    delete observer;
    delete subject;
    return 0;
}
```