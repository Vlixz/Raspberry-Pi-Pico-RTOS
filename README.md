# Raspberry Pi Pico RTOS

I made a simple operating system called RTOS to learn how real-time systems work.

## Features

- **Round-Robin Scheduling**: The RTOS utilizes round-robin scheduling to allocate CPU time among various tasks. The scheduler is invoked by the systick timer every 1ms.
- **Delay Function**: The RTOS includes a delay function that allows tasks to pause execution for a specified number of ticks.
- **Semaphores**: A simple semaphore implementation is provided, allowing tasks to perform operations such as giving and taking.

## Building the Program

This project can be built using PlatformIO.

### Prerequisites

- Install [PlatformIO](https://platformio.org/installation)
- A second Raspberry Pi Pico as a [picoprobe](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf#picoprobe_section) to upload and debug the project

### Building

1. Clone the repository: `git clone https://github.com/your-username/your-repo.git`
2. Open the project in PlatformIO.
3. Build/Upload the project.
