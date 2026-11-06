# ElfPack 3 Architecture

```mermaid
flowchart LR
    subgraph Firmware
        direction TB
        Phone_Firmware[(Phone's Firmware)]
        
    end
```

```mermaid
flowchart LR
    subgraph Client
        UI[Web app]
        Cache[(Local cache)]
    end
    subgraph Services
        API[API gateway]
        Auth[Auth service]
        Orders[Order service]
    end
    subgraph Storage
        DB[(Orders DB)]
    end
    UI --> API
    UI --> Cache
    API --> Auth
    API --> Orders
    Orders --> DB
    Auth -. token .-> UI
```

```mermaid
flowchart LR
    A:::foo & B:::bar --> C:::foobar
    classDef foo stroke:#f00
    classDef bar stroke:#0f0
    classDef foobar stroke:#00f
```

```mermaid
---
title: Mint (Simple)
---
graph TB
    Client --> web[Web Server] --> accounts[Accounts API]
    accounts --> tes[Transaction Extraction Service] --> category[Category Service] & budget[Budget Service] & notif[Notification Service]
    accounts --> db
    tes --> db[SQL]
    tes --> store[Object Store]

%% Style definitions below
    style Client fill:#ff9999,stroke:#ff3333,stroke-width:2px,color:#000
```
