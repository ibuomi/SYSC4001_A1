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
    // Customizable parameters for experiments
    int context_save_time = 10; // Change for analysis
    int isr_activity_time = 40; // Change for analysis
    int current_time = 0;
    // Lambda for logging events
    auto log_event = [](int time, int duration, const std::string& desc) {
        return std::to_string(time) + ", " + std::to_string(duration) + ", " + desc + "\n";
    };
    /******************************************************************/

    // Parse each line of the input trace file
    while (std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        if (activity == "CPU") {
            execution += log_event(current_time, duration_intr, "CPU burst executed");
            current_time += duration_intr;
        } else if (activity == "SYSCALL" || activity == "END_IO") {
            // Interrupt simulation steps
            int device_num = duration_intr;
            // Boilerplate: kernel mode, context save, vector lookup, load ISR
            auto [boilerplate, after_boilerplate_time] = intr_boilerplate(current_time, device_num, context_save_time, vectors);
            execution += boilerplate;
            int time_spent = after_boilerplate_time - current_time;
            current_time = after_boilerplate_time;

            // Device delay from table
            int device_delay = (device_num < delays.size()) ? delays[device_num] : 100;
            int isr_total = device_delay - time_spent - 1; // -1 for IRET
            if (isr_total < isr_activity_time) isr_total = isr_activity_time; // Ensure minimum

            // Unique ISR event wording
            std::string isr_desc = (activity == "SYSCALL") ? "ISR: device driver routine" : "ISR: finalize I/O for device";
            execution += log_event(current_time, isr_total, isr_desc);
            current_time += isr_total;

            // IRET step
            execution += log_event(current_time, 1, "Return from interrupt (IRET)");
            current_time += 1;
        } else {
            // Unknown activity, log for debugging
            execution += log_event(current_time, 1, "Unknown activity: " + activity);
            current_time += 1;
        }
        /************************************************************************/
    }

    input_file.close();
    write_output(execution);
    return 0;
}
