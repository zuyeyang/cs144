Checkpoint 1 Writeup
====================

My name: Benson Zu

My SUNet ID: zuyeyang

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: rqzhang

This lab took me about [7] hours to do. I did attend the lab session.

# Program Structure and Design of the Reassembler:
[
## Attributes of the Reassembler class:
class Reassembler
{
private:
  // /*Helper Function*/
  std::map<uint64_t, std::string> buffer_ {}; /* buffer for reassembler*/
  uint64_t first_unacceptable_index_ = 0;     /* upper bound of the buffer idx*/
  uint64_t first_unassembled_index_ = 0;      /* end of buffered bytes*/
  uint64_t first_unpopped_index_ = 0;         /* next index to be popped*/
  uint64_t capacity_ = 0;                     /* size of buffer got*/
  uint64_t closing_bytes_ = 0;                /* the end of the eof file*/
  bool is_end_received_ = false;              /* If end of file is received*/
}

## Important Design Aspects of `The Reassembler::insert function`

0. Using boundary indices to truncate or filter incoming data
I used the relationship among first unpopped index, first unassembled index 
and first unacceptable index to manage boundary indices, handle 
out-of-boundary data, and truncate data if it extends beyond the available
capacity. Specifically, the first_unpopped_index_ represents the number of
bytes that have been popped, while the first_unassembled_index_ is equal to
the sum of the first_unpopped_index_ and the bytes_pending in the buffer. 
Additionally, the first_unacceptable_index_ is calculated by adding the buffer's
capacity_ to the first_unpopped_index_.

1. Handling overlapping and adjacent data:
The function is designed to handle overlapping or adjacent data efficiently.
It employs the lower_bound function to determine the appropriate position in
the buffer for the new data and carries out merging operations based on the
new data's position relative to the existing elements in the buffer. By
combining this approach with a buffer designed using <uint64_t, std::string>,
the implementation ensures optimal data storage by writing data as continuously
as possible, minimizing the amount of I/O required for this buffer, and
simultaneously conserving memory for later writes.

2. Writing data to the output:
The function writes continuous bytes to the output only when it encounters a
contiguous sequence of data in the buffer. This ensures that data is written
in the correct order and that no gaps are left in the output stream.

3. Closing the bytestream:
The function checks if all data has been pushed, the end-of-file flag is set,
and the buffer is empty. If all these conditions are met, it closes the 
bytestream, signaling that the reassembly process is complete.
]

# Implementation Challenges:
[
The most troublesome part to be striking the right balance between code
complexity and runtime complexity. My original approach involved using 
a map of <uint64_t, char>, which made it easy to write and handle the 
overlapping data cases. However, this approach turned out to be quite 
slow in terms of performance.

To address this issue, I decided to change my strategy and use 
<uint64_t, string> instead. This allowed me to avoid writing data byte
by byte, which significantly improved the runtime performance. However,
this change made handling overlapping data cases more complicated.

I discovered that visualizing the data was extremely helpful in 
understanding the rules for overlapping data situations. I focused
on modifying the writing and popping sections of the assembler buffer
while ensuring that the rest of the logic remained consistent. This approach
proved to be more difficult but yielded a 100 times faster runtime 
performance. In order to debug and test my code, I primarily used tools 
like gdb and CMake Tools from Visual Studio Code. 
]


# Remaining Bugs:
[None]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
