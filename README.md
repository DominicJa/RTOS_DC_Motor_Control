# RTOS_DC_Motor_Control

📌 Description

This project was developed over the course of an academic semester as the main lab assignment for the senior-level Computer Engineering course EGEC 451: Real-Time Operating Systems for Embedded Systems.

The objective of this project was to adapt a rudimentary real-time operating system (RTOS) to understand various RTOS concepts and mechanisms in embedded systems. The project started with a bare-bones RTOS, with each subsequent lab adding more functionality. The repository reflects this iterative development process, with separate entries corresponding to different stages of the project.

🛠 Project Breakdown

(1) Initial Setup & Motor Control

The first phase focused on setting up the EduBase-V2 Trainer's onboard LCD display and interfacing with the TivaC's PWM and timer peripherals:

✅ PWM Peripheral - USed to control the speed of the external DC motor.

✅ Timer Peripheral - Used for input caputre from the motor's encoder, allowing measurement of rotations per minute (RPM)

✅ LCD Display Output - Displayed real-time RPM using timing data from interrupts.

(2) Implementing the RTOS for the first time

The second phase introduced the RTOS given and based on the textbook Real-Time Operating Systems for ARM Cortex-M Microcontrollers by Jonathan W. Valvano.

🔹 Testing & Initializing the RTOS – Implemented the given RTOS framework and verified its functionality.

🔹 Time-Slice Scheduling – Added a time-slice counter to the thread scheduler.

🔹 SPI & Seven-Segment Display Integration – Enabled real-time display of the time-slice counter onto the seven-segment display of the EduBase-V2.

🔹 Thread Control Block (TCB) Enhancements – Added support for thread identifiers, displayed on the seven-segment display.

User Thread Implementation – Three user threads were created:

1️⃣ Motor Speed Control Thread – Adjusted PWM to set motor speed.

2️⃣ RPM Display Thread – Printed real-time motor RPM on the LCD.

3️⃣ Additional Functionality Thread (implemented in later phases).

(3) Adding User Input & Thread Synchronization

This phase introduced user functionality, thread communication, and blocking mechanisms to the RTOS.

🟢 User Input Handling

  Implemented EduBase-V2 push-button controls for user-defined motor speed.

  Push-button presses triggered an interrupt handler, setting a shared mailbox variable used by the motor control thread.

🟢 Thread Synchronization & Blocking

  Modified the TCB, thread scheduler, and kernel to support blocking mechanisms.

  Introduced OS_Wait(uint32_t sig)* and OS_Signal(uint32_t sig)* functions for binary semaphore synchronization.

  Threads attempting to access a resource would block if unavailable, allowing the scheduler to switch to another thread.

  Implemented blocking for PWM access, ensuring the motor stops if resistance is detected.

  🟢 Safety Mechanism

  A third user thread monitored the RPM error margin.

  If the measured RPM deviated significantly from the target RPM, the thread disabled PWM output, effectively stopping the motor.

  PWM was re-enabled only when the user increased the motor speed.

(4) Implementing Priority Scheduling

🔹 Priority Levels Added – The TCB was modified to include a priority field, where a higher value indicates higher priority.

🔹 Thread Priority Assignments:

  Safety Stop Thread – Assigned the highest priority (2)

Motor Control & RPM Display Threads - Assigned priority (1).
🔹 Modified ARM Assembly Code – Updated the scheduler to always execute the highest-priority thread first.
🔹 Round-Robin Execution for Equal Priorities – Threads with the same priority level were scheduled in a round-robin fashion.

🟢 Avoiding Starvation

  To prevent lower-priority threads from being completely starved, a mutex was introduced.

  The input capture handler signaled when new data was available, allowing non-safety threads to run after the safety thread completed.

  This ensured that display updates and motor control adjustments were still executed, even in high-load scenarios.

📄 Additional Resources

For more details, refer to the project lab reports and source code in this repository.

Video demonstrations coming soon.
