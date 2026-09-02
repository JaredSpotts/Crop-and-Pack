import bpy
from bpy.types import Operator
from bpy.types import Panel

bl_info = {
    "name": "Render Helper",
    "blender": (2, 80, 0),
    "category": "Render"
}

#class RENDERPROPERTIES(bpy.types.PropertyGroup):
    

class RENDERHELPER_PT_custom_panel(bpy.types.Panel):
    """Creates side bar panel for the render helper"""
    bl_label = "Render Helper"
    bl_idname = "RENDERHELPER_PT_custom_panel"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Render Helper"
    
    bpy.types.Scene.property.render_pass = bpy.props.IntProperty(
        name = "Render Pass",
        default = 0
        )
    
    def draw(self, context):
        layout = self.layout
        obj = context.object
        scene = context.scene
        layout.label(text="Hello World from your panel!")
        layout.prop(scene, "render_pass")
        
classes = (
    RENDERHELPER_PT_custom_panel, 
)


# registering and menu integration
def register():
    for cls in classes:
        bpy.utils.register_class(cls)
        

# unregistering and removing menus
def unregister():
    for cls in classes:
        bpy.utils.unregister_class(cls)   

if __name__ == "__main__":
    register()

