# SimpleFC-Sim

SimpleFC-Sim is a simple FC (fully connected layer) simulator.

It consists of three main components, which are memory, unified buffer, and mac.

MAC will request for data from unified buffer to use for its computation.

Unified buffer consists of two buffers with same specifications. It is responsible choosing the buffer for bringing in data from memory and sending data to MAC dynamically, respectively.

The memory module receives incoming requests from the unified buffer and sends data accordingly.

The goal for this simple simulator is to check for utilization of different modules via idle and busy cycles, not checking for correctness.
