# ElfPack3 Architecture

```mermaid
flowchart TB
    subgraph Phone_Hardware [Phone's Hardware]
        direction TB
        RAM[(RAM)]
        IRAM[(IRAM)]
        ROM[(ROM)]
        IROM[(IROM)]
    end
    subgraph Original_Firmware [Firmware Original]
        direction TB
        Phones_Firmware[(Phone's Firmware)]
        logger_config_main_task("logger_config_main_task()")
        logger_flusher_task("logger_flusher_task()")
        app_pra_registration_table["app_pra_registration_table[]"]
        APP_CALC_MainRegister("APP_CALC_MainRegister()")
    end
    subgraph Firmware_Patches [Firmware Patches]
        direction TB
        EP3_Reactor_Task_A("EP3_Reactor_Task_A()")
        EP3_Reactor_Task_B("EP3_Reactor_Task_B()")
        EP3_Agent_Main_Register_Entry("EP3_Agent_Main_Register_Entry()")
        EP3_APP_Viewer_Register("EP3_APP_Viewer_Register()")
        subgraph API [API]
            direction TB
            EP3_API_BIN_Load("EP3_API_BIN_Load()")
            EP3_API_View("EP3_API_View()")
            EP3_API_Send_To_Reactor_Core("EP3_API_Send_To_Reactor_Core()")
        end
    end
    ROM .-> Phones_Firmware
    Phones_Firmware --> logger_config_main_task
    Phones_Firmware --> logger_flusher_task
    Phones_Firmware --> app_pra_registration_table
    logger_config_main_task --> EP3_Reactor_Task_A
    logger_flusher_task --> EP3_Reactor_Task_B
    app_pra_registration_table --> EP3_Agent_Main_Register_Entry
    EP3_Agent_Main_Register_Entry --> APP_CALC_MainRegister
    EP3_Agent_Main_Register_Entry --> EP3_APP_Viewer_Register
    EP3_Agent_Main_Register_Entry --> EP3_API_BIN_Load
    EP3_APP_Viewer_Register --> EP3_API_View
    EP3_API_BIN_Load -- ep3.ldr --> RAM
    EP3_Reactor_Task_A .-> EP3_API_Send_To_Reactor_Core
    EP3_API_Send_To_Reactor_Core --> EP3_Reactor_Task_A
    EP3_API_Send_To_Reactor_Core --> EP3_Reactor_Task_B
```

# Samples for copying

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
