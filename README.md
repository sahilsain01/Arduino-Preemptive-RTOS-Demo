
# Arduino Preemptive RTOS Project

This project shows how to run many tasks on an Arduino at the same time using a small custom Real-Time Operating System (RTOS). Normally, Arduino runs one task after another, but this project lets tasks run almost simultaneously by quickly switching between them.

It runs three tasks:

Detects objects with an ultrasonic sensor and switches on an LED fast (highest priority).

Reads temperature and humidity every 2 seconds from a DHT11 sensor and blinks the built-in LED.

Blinks another LED every second as a background task.

A timer interrupts every millisecond and decides which task should run next based on their priority. This way, the most important tasks run immediately, making the system fast and responsive.






## Why This Project?

Modern embedded systems and IoT devices need to handle many things at once — like reading sensors, controlling devices, and more. Typical Arduino programs use a simple loop that can’t efficiently run multiple tasks that need quick reactions.

This project builds a small custom preemptive Real-Time Operating System (RTOS) on an Arduino Uno. It lets multiple tasks run smoothly and on time, handling sensors and devices at the same time without missing anything important.


## How Does It Work?

A timer interrupt stops running tasks regularly to pick the highest priority task that needs to run.

Each task has a small control block storing its details like timing and priority.

The scheduler always chooses the most important task ready to run.

The ultrasonic sensor measures distance; the DHT11 reads environment data.

Tasks print status messages on the serial monitor so you can see what’s happening live.
## Scope

Learn how basic RTOS features (like multitasking and preemption) work on limited hardware.

See how sensors and LEDs can run side by side without blocking each other.

Foundation for more complex real-time embedded projects.

Good practice for understanding RTOS concepts and embedded programming.
## Results

Ultrasonic task interrupts others immediately when detecting nearby objects.

Reliable environmental data every 2 seconds from the DHT11 sensor.

Background LED blinking continues smoothly.

Serial monitor shows timely and orderly task executions.

Proven real-time multitasking working on Arduino Uno with minimal resources.




<img width="688" height="305" alt="Screenshot from 2025-11-03 10-07-20" src="https://github.com/user-attachments/assets/854f2afe-0909-4b84-ac6a-39f1c855fcbc" />

## Conclusion

This project shows how a custom small preemptive RTOS can run on a common Arduino board to handle multiple time-sensitive tasks effectively. It teaches how priority and timer-based scheduling work to keep tasks responsive and efficient.

Learning this helps build skills for designing advanced embedded systems and IoT devices that need multitasking and real-time control.
