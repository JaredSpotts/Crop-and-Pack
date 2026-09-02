# Crop-and-Pack
A Blender-to-Godot sprite atlas pipeline for cropping rendered animation frames, packing aligned render passes, and preparing game-ready 2D assets.

The animations in my game are pre-rendered using 3D models in blender.  They are exported as indivdual frames of animation, but the resource I'm using in the Godot game engine (SpriteFrames) relies on atlases containing each frame of animation on one image, meaning the individual frames need to be stitched into one image.


The tool was built for a workflow where character animations are pre-rendered from 3D models in Blender and imported into Godot as 2D sprite animations. Blender renders each animation as a sequence of individual image frames, while Godot's `SpriteFrames` resource relies on those frames are packed into atlas textures. Crop-and-Pack automates that conversion, while also using the pipeline to crop unused transparent space, preserve alignment across multiple render passes, and make a larger number of animations practical to include in the game.
