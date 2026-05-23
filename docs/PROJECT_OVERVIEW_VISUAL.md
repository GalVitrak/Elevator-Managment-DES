# Visual Overview — Diagrams for Slides

All diagrams use **Mermaid**. Paste into GitHub, VS Code preview, or [mermaid.live](https://mermaid.live) to export PNG/SVG for slides.

---

## 1. System context (Context diagram)

```mermaid
C4Context
    title Elevator DES - System Context
    Person(user, "Operator", "Student /演示者")
    System(sim, "DES Simulator", "C console application")
    System_Ext(log, "simulation_log.txt", "Audit log")
    System_Ext(cfg, "config.txt", "Parameters")

    Rel(user, sim, "Menu input")
    Rel(sim, log, "Writes")
    Rel(sim, cfg, "Load/Save")
```

*If C4 fails in your renderer, use the flowchart in §2 instead.*

---

## 2. Module dependency

```mermaid
flowchart TB
    subgraph UI
        MAIN[main.c]
    end
    subgraph Core
        SIM[simulation.c]
        EVT[event.c]
    end
    subgraph Domain
        ELV[elevator.c]
        FLR[floor.c]
        PSG[passenger.c]
    end
    subgraph Infra
        LOG[logger.c]
        FM[file_manager.c]
        CONST[constants.h]
    end

    MAIN --> SIM
    MAIN --> FM
    MAIN --> LOG
    SIM --> EVT
    SIM --> ELV
    SIM --> FLR
    SIM --> PSG
    SIM --> LOG
    FM --> LOG
    ELV --> LOG
    SIM --> CONST
    FM --> CONST
```

---

## 3. DES execution flow

```mermaid
flowchart TD
    A[Start simulation_run] --> B{FEL empty?}
    B -->|yes| Z[End]
    B -->|no| C{time < maxTime?}
    C -->|no| Z
    C -->|yes| D[pop_earliest event]
    D --> E[currentTime = event.time]
    E --> F[dispatch handler]
    F --> G[log handled]
    G --> H[free event]
    H --> B
```

---

## 4. Entity-relationship (conceptual)

```mermaid
erDiagram
    SIMULATION ||--o{ ELEVATOR : contains
    SIMULATION ||--o{ FLOOR : contains
    SIMULATION ||--|| EVENT_LIST : owns
    FLOOR ||--o{ PASSENGER : queues
    EVENT_LIST ||--o{ EVENT : schedules
    ELEVATOR ||--o| PASSENGER : carries_phase1

    SIMULATION {
        double currentTime
        int numFloors
        int numElevators
    }
    ELEVATOR {
        int id
        int currentFloor
        int status
    }
    EVENT {
        double time
        string type
    }
```

---

## 5. Building side view (conceptual)

```text
     Floor 4  [  queue: · ·  ]  ▲ hall up
     Floor 3  [  queue: ·     ]  ▼
     Floor 2  [  queue:        ]
     Floor 1  [  queue: · · ·  ]
     Floor 0  [  queue: ·     ]  ← ground
              ═════════════════
              │ Elevator 0 │  idle @ floor 0
              │ Elevator 1 │  moving ↑
```

Use in slides to explain **floor queues** vs **elevators**.

---

## 6. FEL linked list structure

```mermaid
flowchart LR
    HEAD((head)) --> E1["t=0.0 CALL"]
    E1 --> E2["t=0.0 ARRIVE"]
    E2 --> E3["t=0.5 CLOSE"]
    E3 --> NULL((null))
```

Insert new event `t=0.2` → scan until `0.2 < next.time`, splice in.

---

## 7. Floor queue (FIFO)

```mermaid
flowchart LR
    F[Floor.waitingQueueFront] --> P1[Passenger 1]
    P1 --> P2[Passenger 2]
    P2 --> P3[Passenger 3]
    R[waitingQueueRear] -.-> P3
```

`enqueue`: append at rear. `dequeue`: remove front.

---

## 8. Passenger state machine

```mermaid
stateDiagram-v2
    [*] --> WAITING: create + enqueue
    WAITING --> IN_ELEVATOR: doors open, board
    IN_ELEVATOR --> ARRIVED: exit event
    ARRIVED --> [*]: destroy
```

---

## 9. Elevator status (phase 1 usage)

```mermaid
stateDiagram-v2
    [*] --> IDLE
    IDLE --> MOVING: assign / travel
    MOVING --> IDLE: doors open complete
    IDLE --> MAINTENANCE: phase 2 only
    IDLE --> OUT_OF_SERVICE: phase 2 only
```

---

## 10. Full request sequence (swimlane)

```mermaid
sequenceDiagram
    autonumber
    actor Op as Operator
    participant Main as main.c
    participant Sim as simulation.c
    participant FEL as event.c
    participant Fl as floor.c
    participant El as elevator.c

    Op->>Main: Add request 0→3
    Main->>Sim: simulation_add_passenger_request
    Sim->>Fl: enqueue passenger
    Sim->>FEL: schedule PASSENGER_CALL @0

    Note over Sim: simulation_run starts
    Sim->>FEL: pop CALL
    Sim->>El: find idle, assign
    Sim->>FEL: schedule ARRIVAL @0
    Sim->>FEL: pop ARRIVAL
    Sim->>FEL: schedule DOORS_OPEN
    Sim->>Fl: dequeue, board
    Sim->>FEL: schedule DOORS_CLOSE @0.5
    Note over Sim: ... continues until FEL empty
```

---

## 11. Phase 1 vs Phase 2 (roadmap visual)

```mermaid
timeline
    title Project timeline
    section Phase 1 Done
        Architecture : modules
        FEL : sorted list
        Handlers : skeleton
        Docs : presentation pack
    section Phase 2 Planned
        Movement : travel delay
        Dispatch : SCAN / nearest
        Stats : reports
        Emergency : maintenance mode
```

---

## 12. Export tips for PowerPoint

1. Open https://mermaid.live  
2. Paste diagram code  
3. Export PNG (transparent background optional)  
4. Insert image per slide  

For ASCII diagrams (§5), use monospace font or screenshot from terminal.
