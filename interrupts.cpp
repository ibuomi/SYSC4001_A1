/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts.hpp>

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/
    int current_time = 0;
    int context_save_time = 10;  // Context switch time in ms (modify for experiments)
    
    // Statistics tracking
    int total_cpu_time = 0;
    int total_interrupt_overhead = 0;
    int total_isr_execution = 0;
    int interrupt_count = 0;
    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/

if (activity == "CPU") {
            // CPU burst - normal execution in user mode
            execution += std::to_string(current_time) + ", " + 
                        std::to_string(duration_intr) + ", CPU execution\n";
            current_time += duration_intr;
            total_cpu_time += duration_intr;
        }
        else if (activity == "SYSCALL") {
            // System call interrupt - initiates I/O operation
            int device_num = duration_intr;
            interrupt_count++;
            
            // Execute interrupt boilerplate (mode switch, save context, find vector, load ISR)
            auto [intr_exec, new_time] = intr_boilerplate(current_time, device_num, 
                                                          context_save_time, vectors);
            execution += intr_exec;
            total_interrupt_overhead += (new_time - current_time);
            current_time = new_time;
            
            // Execute ISR body - device driver initiates I/O
            int isr_duration = delays[device_num];
            execution += std::to_string(current_time) + ", " + std::to_string(isr_duration) 
                        + ", SYSCALL: run the ISR\n";
            current_time += isr_duration;
            total_isr_execution += isr_duration;
            
            // Return from interrupt (IRET)
            execution += std::to_string(current_time) + ", " + std::to_string(1) + ", IRET\n";
            current_time++;
            total_interrupt_overhead++;
        }
        else if (activity == "END_IO") {
            // I/O completion interrupt - device signals operation finished
            int device_num = duration_intr;
            interrupt_count++;
            
            // Execute interrupt boilerplate
            auto [intr_exec, new_time] = intr_boilerplate(current_time, device_num, 
                                                          context_save_time, vectors);
            execution += intr_exec;
            total_interrupt_overhead += (new_time - current_time);
            current_time = new_time;
            
            // Execute ISR body - device driver handles I/O completion
            int isr_duration = delays[device_num];
            execution += std::to_string(current_time) + ", " + std::to_string(isr_duration) 
                        + ", END_IO: run the ISR\n";
            current_time += isr_duration;
            total_isr_execution += isr_duration;
            
            // Return from interrupt (IRET)
            execution += std::to_string(current_time) + ", " + std::to_string(1) + ", IRET\n";
            current_time++;
            total_interrupt_overhead++;
        }

        /************************************************************************/

    }

    input_file.close();

    execution += "\n=== SIMULATION SUMMARY ===\n";
    execution += "Total simulation time: " + std::to_string(current_time) + " ms\n";
    execution += "Total CPU time: " + std::to_string(total_cpu_time) + " ms (" 
                + std::to_string(100.0 * total_cpu_time / current_time) + "%)\n";
    execution += "Total interrupt overhead: " + std::to_string(total_interrupt_overhead) + " ms ("
                + std::to_string(100.0 * total_interrupt_overhead / current_time) + "%)\n";
    execution += "Total ISR execution: " + std::to_string(total_isr_execution) + " ms ("
                + std::to_string(100.0 * total_isr_execution / current_time) + "%)\n";
    execution += "Number of interrupts: " + std::to_string(interrupt_count) + "\n";
    execution += "Average time per interrupt: " 
                + std::to_string((total_interrupt_overhead + total_isr_execution) / (double)interrupt_count) + " ms\n";


    write_output(execution);

    return 0;
}
