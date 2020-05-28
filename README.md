# Godot-Slicer

A port of [Ezy-Slicer](https://github.com/DavidArayan/ezy-slice) for the [Godot game engine](https://godotengine.org/).

## About
Built as a Godot module in C++, Slicer is a port of [David Arayan's Ezy-Slicer](https://github.com/DavidArayan/ezy-slice) Unity plugin (who deserves all credit). It allows for the dynamic slicing of convex meshes along a plane. Built against Godot version 3.2.1.

## Installing
Slicer follows the installation procedure defined in the [Godot custom module documentation guide](https://docs.godotengine.org/en/stable/development/cpp/custom_modules_in_cpp.html). It can be built into a compilation of the engine by cloning the repo into Godot's modules folder:

```bash
git clone git@github.com:cj-dimaggio/godot-slicer.git <godot-repo>/modules/slicer
```

## Using
After building Godot with the Slicer module a `Slicer` node will now be available under `Spatial`. A `Slicer` instance can then be used to trigger slices of `Mesh` geometry like so:

```gdscript
extends RigidBody

class_name Sliceable

export(Material) var cross_section_material
export(Mesh) var mesh_override

func _ready():
	if mesh_override:
		$MeshInstance.mesh = mesh_override

	# Setup the collision shape to be the mesh's shape
	var shape = ConvexPolygonShape.new()
	shape.points = $MeshInstance.mesh.surface_get_arrays(0)[Mesh.ARRAY_VERTEX]
	shape.margin = 0.015
	var owner_id = self.create_shape_owner(self)
	self.shape_owner_add_shape(owner_id, shape)

func cut(plane_origin: Vector3, plane_normal: Vector3):
	var sliced: SlicedMesh = $Slicer.slice($MeshInstance.mesh, self.transform, plane_origin, plane_normal, cross_section_material)

    if not sliced:
        print("No slice occurred")
    
    if sliced.upper_mesh:
        print("Instantiate the upper cut mesh somewhere")

    if sliced.lower_mesh:
        print("Instantiate the lower cut mesh somewhere")
```

An example project can also be found at: https://github.com/cj-dimaggio/godot-slicer


## Development
For development purposes, Slicer can be built as a dynamic library by passing in the `slicer_shared=no` option to SCons and using the `slicer-shared` build alias, such as:

```bash
scons slicer_shared=yes slicer-shared
```

which will build a new dynamic library artifact into Godot's `./bin` folder.

There is also a test suite that can be built and run on Unix systems using:

```bash
scons platform=osx slicer_tests=yes
```

For more information on the testing framework and development see the [corresponding readme](./tests/README.md).