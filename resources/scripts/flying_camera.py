import gamify
from gamify import python_component as pycomponent
from gamify import input_system as input
from gamify import dvec2 as vec2
from gamify import dvec3 as vec3


class flying_camera(pycomponent):
    name = "flying_camera"
    movement_speed = .1
    rotation_speed = .01
    rotation = vec2()

    def __init__(self, gobj):
        pycomponent.__init__(self, self.name, gobj)

    def on_init(self):
        initial_rotation = self.transform().rotation().euler_angles()
        self.rotation = vec3(initial_rotation.y, initial_rotation.x, 0)

    def on_update(self):
        tr = self.transform()
        input = self.get("camera_input")

        if not input:
            return

        diff = input.move
        updown = input.updown
        movement = (tr.right() * diff.x + tr.forward()
                    * diff.y + tr.up() * updown) * self.movement_speed
        tr.move(movement)

        rot_diff = input.rotate
        self.rotation.x -= rot_diff.x * self.rotation_speed
        self.rotation.y -= rot_diff.y * self.rotation_speed
        tr.set_rotation(vec3(self.rotation.y, self.rotation.x, 0))

    def on_deinit(self):
        pass


module_exports = {flying_camera}
