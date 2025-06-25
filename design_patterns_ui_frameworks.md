# Design Patterns for UI Frameworks

## Subtopics

### Model-View-Controller (MVC) and Model-View-ViewModel (MVVM) in Qt
- **What they are:** Architectural patterns that separate the application's data and business logic from its user interface. This separation makes the application easier to test, maintain, and evolve.
- **MVC Breakdown:**
    - **Model:** Manages the application's data and business logic. It knows nothing about the UI. In Qt, this is often a `QAbstractItemModel` subclass.
    - **View:** Displays the data from the Model. It knows how to present the data but not what the data is or how to manipulate it. In Qt, this is a `QAbstractItemView` subclass (like `QListView`, `QTableView`).
    - **Controller:** Takes user input, manipulates the Model, and causes the View to update. In a classic Qt Widgets application, the `QWidget` or `QMainWindow` often acts as both the View and the Controller.
- **MVVM Breakdown:** A refinement of MVC, particularly popular in modern UI frameworks like QML.
    - **Model:** Same as in MVC (data and business logic).
    - **View:** The UI layout, often defined declaratively (e.g., in a QML file). It's responsible for display only. It binds directly to the ViewModel.
    - **ViewModel:** A specialized Controller that acts as a data converter and state manager for the View. It exposes data from the Model as properties and commands that the View can bind to. This eliminates the need for the View to have complex logic, making it very "dumb". The ViewModel knows nothing about the specific UI controls in the View.
- **Why it's useful in UI/Qt:** Qt's Model/View architecture is a direct implementation of these principles. It allows you to display the same data in completely different ways (e.g., a list, a table, a tree) without changing the underlying data model. MVVM is the cornerstone of QML, enabling clean separation between the C++ backend (Model/ViewModel) and the QML frontend (View).
#### Simple Example: ViewModel in Qt (C++/QML)

A minimal ViewModel in Qt is a QObject-derived C++ class that exposes properties and signals for QML to bind to. Here's a basic example:

**C++ ViewModel (MyViewModel.h/.cpp):**
[Header](code_examples/my_view_model.h) | [Implementation](code_examples/my_view_model.cpp)

### Observer Pattern and Qt's Signal/Slot System
- **What it is:** A behavioral pattern where an object, called the "subject," maintains a list of its dependents, called "observers," and notifies them automatically of any state changes.
- **How it works:**
    - The Subject provides an interface to attach and detach Observer objects.
    - When the Subject's state changes, it iterates through its list of Observers and calls a specific notification method on each one.
    - Observers can be added or removed at any time.
- **Why it's useful in UI/Qt:** This is the fundamental pattern for creating decoupled and event-driven systems. UI elements need to react to changes in data models or user actions without being tightly coupled to them. Qt's **signals and slots** mechanism is a type-safe, powerful implementation of the Observer pattern.
    - **Signal (Subject):** An object emits a signal when its state changes (e.g., `QPushButton::clicked()`). It doesn't know or care what, if anything, is listening.
    - **Slot (Observer):** A function that is called in response to a specific signal.
    - **Connection:** `QObject::connect()` is used to attach a signal to a slot. This creates the subscription.
#### How does Qt's Observer (Signal/Slot) differ from the Publisher-Subscriber (e.g., DDS) pattern?

While Qt's signal/slot system is an implementation of the Observer pattern, there are important differences compared to the Publisher-Subscriber (Pub/Sub) pattern as seen in middleware like DDS (Data Distribution Service):

- **Tight vs. Loose Coupling:**
    - **Qt Signal/Slot:** Connections are typically made directly between objects within the same process. The sender (signal) and receiver (slot) are both `QObject` instances, and the connection is established explicitly via `QObject::connect()`. This is a form of *tight coupling* at the object level, though the sender does not need to know the receiver's details.
    - **Pub/Sub (DDS):** Publishers and subscribers are *decoupled* in space, time, and synchronization. They do not need to know about each other's existence, and can be in different processes, machines, or even networks. Communication is mediated by a broker or middleware.
**Q: If you state for the above, that objects are in the same process in Qt, seems like their signals can be sent from different threads – right or not? How is inter-thread communication handled in such case?**

**A:**  
Yes, in Qt, signals can be emitted from one thread and received in another, as long as both sender and receiver are `QObject`-derived and live in different threads (i.e., have different thread affinities). Qt's signal/slot system is designed to handle both intra-thread and inter-thread communication safely.

**How does Qt handle inter-thread signal/slot connections?**

- **Connection Types:**  
  When you connect a signal to a slot, Qt determines the connection type:
    - **Direct Connection:** If the sender and receiver live in the same thread, the slot is called immediately, just like a normal function call.
    - **Queued Connection:** If the sender and receiver live in different threads, Qt uses a *queued connection*. The signal is placed in the event queue of the receiver's thread, and the slot is executed later by the receiver's thread event loop.
    - **Auto Connection (default):** Qt automatically chooses direct or queued connection based on thread affinity.

- **Thread Affinity:**  
  Each `QObject` has a thread affinity (the thread it "belongs" to). You can move a QObject to another thread using `QObject::moveToThread()`.

- **Event Loop Requirement:**  
  For queued connections to work, the receiver's thread must have a running Qt event loop (typically via `QThread::exec()`).

- **Data Passing:**  
  Signal arguments are copied (using the meta-object system) when crossing threads, so you should avoid passing non-copyable or large objects by value.

**Example:**

- **Communication Scope:**
    - **Qt:** Signal/slot is designed for *intra-process* communication (though queued connections can cross threads). It is not inherently networked or distributed.
    - **DDS:** Pub/Sub is designed for *inter-process* and *distributed* communication, often across networks, supporting scalability and robustness in large systems.

- **Discovery and Subscription:**
    - **Qt:** You must explicitly connect a signal to a slot in code. There is no automatic discovery.
    - **DDS:** Subscribers can discover publishers dynamically at runtime based on topics and Quality of Service (QoS) policies.

- **Data Model:**
    - **Qt:** Signals and slots are function signatures; data is passed as function arguments.
    - **DDS:** Data is published to *topics*; subscribers receive data for topics they are interested in, often with rich data types and serialization.

- **Reliability and QoS:**
    - **Qt:** No built-in support for delivery guarantees, message persistence, or QoS.
    - **DDS:** Provides extensive QoS controls (reliability, durability, ordering, deadlines, etc.) for real-time and mission-critical systems.

**Summary Table:**

| Aspect                | Qt Signal/Slot (Observer) | DDS Pub/Sub (Publisher-Subscriber) |
|-----------------------|---------------------------|-------------------------------------|
| Coupling              | Tight (object-level)      | Loose (topic-based)                 |
| Communication         | Intra-process/thread      | Inter-process/networked             |
| Discovery             | Manual connect            | Dynamic, topic-based                |
| Data Model            | Function arguments        | Structured topics                   |
| QoS/Delivery          | None                      | Extensive QoS options               |

**In short:**  
Qt's signal/slot is ideal for decoupling components *within* an application, while Pub/Sub (like DDS) is designed for scalable, distributed, and robust communication *between* applications or systems.

---

Would you like a code example showing how a DDS Pub/Sub system might look compared to Qt's signal/slot, or a deeper dive into distributed observer patterns?

### Command Pattern for Undo/Redo in UI
- **What it is:** A behavioral pattern that turns a request into a stand-alone object containing all information about the request. This includes the method to call, the object that owns the method, and the values for the method parameters.
- **How it works for Undo/Redo:**
    - Create a base `Command` class with `execute()` and `undo()` methods.
    - For each user action (e.g., "delete item," "change color"), create a concrete command class that implements these methods.
    - When the user performs an action, you instantiate the corresponding command object and call `execute()`.
    - You then push this command object onto a "history" or "undo" stack.
    - To perform an **undo**, you pop the last command from the stack and call its `undo()` method. You might move it to a "redo" stack.
    - To perform a **redo**, you pop from the redo stack, call `execute()` again, and push it back onto the undo stack.
- **Why it's useful in UI/Qt:** It provides a clean and extensible way to implement complex user action histories. It decouples the code that invokes an operation from the code that knows how to perform it. Qt provides the `QUndoStack`, `QUndoCommand`, and `QUndoView` classes to make implementing this pattern straightforward.

### State Pattern for UI State Management
- **What it is:** A behavioral pattern that allows an object to alter its behavior when its internal state changes. The object appears to change its class.
- **How it works:**
    - Define a `State` interface with methods for all possible actions (e.g., `handlePlay()`, `handleStop()`).
    - Create concrete state classes (e.g., `PlayingState`, `PausedState`, `StoppedState`) that implement the interface. Each implementation defines the behavior specific to that state.
    - The main object (the "Context," e.g., a `MediaPlayer` widget) holds a pointer to a current state object.
    - When an action is requested on the Context, it delegates the call to its current state object.
    - The state objects themselves are responsible for transitioning the Context to a new state (e.g., when `PlayingState` handles a `pause` action, it sets the Context's current state to a new `PausedState` object).
- **Why it's useful in UI/Qt:** It elegantly solves the problem of managing a UI component that has complex states, avoiding massive `if/else` or `switch` statements. It's perfect for wizards, connection dialogs (connecting, connected, failed, disconnected), or any component whose available actions depend on its current mode.

### Factory and Abstract Factory for UI Component Creation
- **What they are:** Creational patterns that provide an interface for creating objects in a superclass, but let subclasses alter the type of objects that will be created.
- **Factory Method:** Defines an interface for creating a single object, but lets subclasses decide which class to instantiate.
    - **Use Case:** You have a framework for creating documents. A base `Application` class has a `createDocument()` factory method. A `TextEditorApplication` subclass overrides it to return a `TextDocument`, while a `DrawingApplication` returns a `DrawingDocument`.
- **Abstract Factory:** Provides an interface for creating *families* of related or dependent objects without specifying their concrete classes.
    - **Use Case:** You need to support multiple UI themes (e.g., "Light," "Dark," "Classic"). You define an `AbstractWidgetFactory` with methods like `createButton()`, `createScrollBar()`, `createWindow()`. You then have concrete factories like `DarkThemeFactory` and `LightThemeFactory` that create widgets styled for that specific theme. The application code uses the abstract factory, so switching themes is as simple as swapping out the concrete factory instance.
- **Why it's useful in UI/Qt:** Essential for building extensible and themeable UI frameworks. It allows you to decouple the client code from the concrete classes of the UI elements it needs to create, promoting flexibility and adherence to the Open/Closed Principle.

### Strategy Pattern for Runtime UI Behavior Changes
- **What it is:** A behavioral pattern that enables selecting an algorithm at runtime. It defines a family of algorithms, encapsulates each one, and makes them interchangeable.
- **How it works:**
    - Define a `Strategy` interface that is common to all supported algorithms.
    - Create concrete strategy classes that implement this interface.
    - The "Context" class (e.g., a `Validator` or `Layouter`) is configured with a concrete strategy object and maintains a reference to it.
    - The Context delegates the work to the strategy object instead of implementing the behavior itself. The strategy can be changed at any time.

There is a strong relationship between the Strategy pattern (as described above) and dependency injection (DI).

**How they relate:**
- The Strategy pattern relies on configuring a "Context" class with a concrete strategy object that implements a common interface. The Context delegates work to this strategy.
- Dependency injection is a technique for providing (injecting) dependencies (like a strategy object) into a class, rather than having the class construct them itself.

**In practice:**
- When you use the Strategy pattern, you often want to be able to swap out the strategy at runtime, or at least configure it at construction time. Instead of the Context class creating a specific strategy internally (tight coupling), you inject the desired strategy from outside—either via the constructor, a setter, or a DI framework.
- This makes the Context class more flexible, testable, and decoupled from specific strategy implementations.

**Example (C++/Qt style):**
- **Why it's useful in UI/Qt:**
    - **Layout Management:** A container widget could use different layout strategies (`HorizontalLayoutStrategy`, `VerticalLayoutStrategy`, `GridLayoutStrategy`) that could be swapped.
    - **Validation:** An input field can be configured with different validation strategies (`EmailValidatorStrategy`, `PhoneNumberValidatorStrategy`, `NotEmptyValidatorStrategy`).
    - **Data Sorting/Filtering:** A view can use different sorting strategies (sort by name, sort by date, etc.) by simply swapping the strategy object.

### Composite Pattern for Hierarchical UI Elements
- **What it is:** A structural pattern that lets you compose objects into tree structures to represent part-whole hierarchies. Composite lets clients treat individual objects and compositions of objects uniformly.
- **How it works:**
    - Define a common `Component` interface for both simple objects ("Leafs") and complex objects ("Composites"). This interface includes operations applicable to all parts, like `draw()` or `getBounds()`.
    - "Leaf" classes are the individual building blocks of the hierarchy (e.g., a `Button`, a `Label`). They implement the `Component` interface but have no children.
    - "Composite" classes are containers (e.g., a `Panel`, a `Window`). They also implement the `Component` interface, but their methods typically delegate the work to their child components. They also have methods for adding, removing, and accessing children.
- **Why it's useful in UI/Qt:** This is the absolute foundation of how graphical user interfaces are structured. In Qt, `QWidget` is a perfect example of the Composite pattern. A `QWidget` can be a simple leaf (like a `QPushButton`) or a composite that contains other `QWidget`s (like a `QFrame` or `QMainWindow`). You can call `update()` or `hide()` on a top-level widget, and the action propagates down the entire hierarchy uniformly.

---
**IF TIME ALLOWS:**  
### Singleton Pattern: Pros, Cons, and Alternatives in Qt
- **What it is:** A creational pattern that ensures a class has only one instance and provides a global point of access to it.
- **Pros:**
    - **Guaranteed Single Instance:** Useful for managing a shared resource, like a database connection, logging service, or application settings manager.
    - **Global Access:** Easy to access from anywhere in the codebase.
- **Cons:**
    - **Global State:** It's essentially a global variable, which can lead to hidden dependencies and make code hard to reason about.
    - **Difficult to Test:** Tightly couples code to the concrete singleton class, making it difficult to mock or stub for unit testing.
    - **Concurrency Issues:** Requires careful handling in a multithreaded environment to prevent race conditions during instantiation.
- **Alternatives in Qt:**
    - **Dependency Injection:** The preferred modern approach. Create the "single" object (e.g., `SettingsManager`) in `main()` and pass it down through constructors or initializers to the objects that need it. This makes dependencies explicit and testing easy.
    - **Service Locators:** A central registry where objects can request services instead of creating them. Less ideal than DI but better than a classic Singleton.
    - **Using `QCoreApplication::instance()`:** For truly application-wide singletons, you can subclass `QApplication`, add your manager objects to it, and access them via a static getter that retrieves the application instance. Example: `static_cast<MyApplication*>(QCoreApplication::instance())->settingsManager()`.

### Adapter and Proxy Patterns for Legacy Integration
- **Adapter Pattern:**
    - **What it is:** A structural pattern that allows objects with incompatible interfaces to collaborate. It acts as a wrapper or translator between two different interfaces.
    - **Why it's useful:** Imagine you have a third-party C-style library for data processing that uses callback functions. You want to integrate it into your Qt application. You can write an `Adapter` class that inherits from `QObject`, exposes Qt signals, and internally calls the C library's functions and handles its callbacks. This "adapts" the callback mechanism to the signal/slot mechanism.
- **Proxy Pattern:**
    - **What it is:** A structural pattern that provides a surrogate or placeholder for another object to control access to it.
    - **Why it's useful:**
        - **Virtual Proxy:** For lazy loading. Imagine a UI with many high-resolution image thumbnails. Instead of loading all images at once, you can use a `ProxyImage` object. It has the same interface as the real image object but initially contains only the path. The full image is only loaded from disk when the `draw()` method is actually called.
        - **Protection Proxy:** To control access rights. A proxy can check if the calling object has the necessary permissions before forwarding a request to the real object.
        - **Qt Example:** `QSortFilterProxyModel` is a classic proxy. It doesn't hold the data itself; it sits between a view and a source model, providing a different view of the data (sorted or filtered) while forwarding data requests to the original source model.