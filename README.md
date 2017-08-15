# mapedit-old

This is the source code for my old map-editing application "mapedit".
It runs only on an X11 window. The goal was to create a map-editing application.

However, this goal was not achievable in C++, especially regarding reproducable
builds. I have since converted this project to Rust (private). 

It remains as the best C++ I have written so far in a non-hello-world project.

Looking back - the goals were very different, because I didn't know what I was
doing. I wanted to integrate it with the AWS SDK to analyze OSM data - it turned
out this was completely unnecessary. I wanted to integrate it with OpenCL, to make
it run faster - this was unneccessary, too.

The project uses a lot of C-style pointers. Which is not recommended at all, but 
at the time I didn't know better. At best you can get this project to render a 
few strings on the screen (it renders a font character by character).

I have since improved the code (in Rust), it renders font atlasses and it's much more 
efficient in terms of draw calls.

I have abandoned this project because I simply got nowhere. Yes, I could link things
together, but I didn't make any progress. Rust (and its package manager cargo) have
solved these problems for me and they run reasonably fast.

It was the first non-trivial codebase I've written. It's just a showcase that 
"yes, I have written C++".
