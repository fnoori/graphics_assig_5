## Development Environment
* Device --- MacBook Pro
* OS --- MacOS highSierra 10.13.3
* IDE --- XCode (9.2 (9C40b))
* OpenGL --- 4.1
----------------------------------
## Controls
| Button        | Function           |
| ------------- |:-------------:|
| `Left mouse click`| Hold down and move mouse to move the spherical camera | $1600 |
| `Mouse scroll wheel`      | Zoom in and out      |
| `W` | Speed up animation      |
| `S` | Slow down animation |
| `P` | Pause animation |
| `O` | Start animation |

## Part I: A Sphere
* Spheres render correctly
###### Part I (Limitations)
* n/a

## Part II: A Spherical Camera
* Camera always looks to the centre of the scene
###### Part II (Limitations)
* n/a

## Part III: A Scene Graph
* All celestial bodies orbit and rotate appropriately
* The earth is correctly titled on its axis of rotation
* The earth's moon orbits the earth; with respect with the earth's axis of rotation
* Backdrop of stars (and the milky way) dispaly correctly, even when moving the camera
###### Part 3 (Limitations)
* n/a


## Part IV: Texturing & Shading
* Textures are correctly applied to the spheres
###### Part IV (Limitations)
* Shading does not work correctly
* The specular lighting follows the camera, as opposed to following the sun


## Part V: Animation
* Animation of all the celestial bodies work correctly
* Animation can be sped up or slowed down using the controls mentioned above
* Animation can be paused and started
###### Part V (Limitations)
* Animations are not "to scale"

## References
| Link        | Use           |
| ------------- |:-------------:|
| https://www.solarsystemscope.com/textures/| All the textures | $1600 |
| https://github.com/SonarSystems/Modern-OpenGL-Tutorials/blob/master/%5BLIGHTING%5D/%5B12%5D%20Point%20Light/res/shaders/lighting.frag      | Failed attempt at shading, with guidence from this link      |
