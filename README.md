# Vulkan Physics Engine

Classical mechanics was always my favorite physics subject, and I also like video games, so
I thought it would be a good idea to tackle a physics engine for 3d games and simulation.

Little did I know that this would need me to learn (and relearn) a ton of math, physics, and computer
science topics. Luckily for me, C++, linear algebra, differential equations, and physics intuition were all
mostly still intact from previous experiences.

This project is different from my other ones. Whereas those are mostly case studies/proof of concept builds,
this project is designed to help me grow in the world of real time physics and graphics. I named this "Vulkan Engine" because
I plan on expanding it past physics, but for now, rigid body dynamics is the goal. I'm new to github so some parts of the repo
may not be super duper clean but I hope to get better at that too.

# Tools

The tools I'll be using are Vulkan (obviously), C++ (obviously), and Slang for shading. I chose this stack more or less because
its relatively modern compared to most physics sims. 

The ones I see online tend to use OpenGL but since I want to later extrapolate this project to computer graphics 
and a full engine, I decided to go with Vulkan. 

I chose C++, but if you see my code, you'll notice its very C like. This is deliberate. I initially started writing it with 
modern C++ like vulkan tutorial says to, but I ended up scrapping it because I found myself having too much friction between 
my math and ideas with the actual language. Nothing wrong with modern C++, I  actually like it for certain projects, 
just not this one. Im going with C API as the header of choice. I decided against RAII for this project because it doesnt fit the mental
model for physics or graphics in this particular case. I'd love to hear on how raii has helped your graphics projects, but for this one, I'm
sticking with very data oriented design and C style code. 

I chose Slang for shading because it was the only language I knew and the syntax is very C like so its much less context switching. Also it 
just feels a bit more modern than something like GLSL.

I do like programming on the go so my device is M3 Macbook Pro. I am using the new vulkan drivers for mac and will eventually rewrite some of the
code to reflect that. I initially thought I was on moltenVK but it turns out that I had a more modern SDK. I can remove some of the things that
were moltenVK specific and this project should be able to run on any device. I also have a windows desktop that is more powerful than this laptop 
and I'll likely switch development environments to it when the project warrants the change.

# Last Thoughts

I'll try my best to cite all the resources that I'll be using, but since a ton of the project is learning, its unreasonable to expect every single 
link for all the resources. But I'll do my best to share the big ones.

I'm excited to go through this journey. As you can see from the way I write + my descriptions and bio, I don't really take this github 
too seriously. It is just a place to show off my work and I'm kind of against embellishing and overstating the things I do. The documentation and 
blogging of this will be pretty straight forward. If you have read my commits, you'll likely see that.
