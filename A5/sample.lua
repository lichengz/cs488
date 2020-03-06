stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0, 0)
grass = gr.material({0.5, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0, 0)
lake_blue = gr.material({0, 0.3, 0.3}, {0.0, 0.0, 0.0}, 0, 0.5)
cow_white = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 0, 0)
stone_2 = gr.material({0.4, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0, 0)
waterball_blue = gr.material({0.0, 0.5, 0.8}, {0.8, 0.8, 0.8}, 20, 0.5)

-- ##############################################
-- the stone gate
-- ##############################################


gate = gr.node('gate')
gate:translate(0, 0, 0)

p1 = gr.mesh('icosa', './Assets/icosa.obj')
gate:add_child(p1)
p1:set_material(stone)
p1:scale(0.8, 4, 0.8)
p1:translate(-2.0, 0, 0)

p2 = gr.mesh('icosa', './Assets/icosa.obj')
gate:add_child(p2)
p2:set_material(stone)
p2:scale(0.8, 4, 0.8)
p2:translate(2.0, 0, 0)

s = gr.mesh('icosa', './Assets/icosa.obj')
gate:add_child(s)
s:set_material(stone)
s:scale(4, 0.8, 0.8)
s:translate(0, 3.5, 0)

-- ##############################################
-- the scene
-- ##############################################

scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor

plane = gr.mesh('plane', './Assets/plane.obj' )
scene:add_child(plane)
plane:set_material(grass)
plane:scale(30, 30, 30)

-- the lake
lake = gr.mesh( 'lake', './Assets/buckyball.obj' )
scene:add_child(lake)
lake:set_material(lake_blue)
lake:scale(5, 0.1, 5)

-- the sun
waterball = gr.nh_sphere('waterball', {0, 10, 0}, 3)
scene:add_child(waterball)
waterball:set_material(waterball_blue)


-- the cube
cube = gr.nh_box('cube', {0, 0, 0}, 1)
scene:add_child(cube)
cube:set_material(stone_2)
cube:scale(3, 3, 3)
cube:translate(-8.5, 0, 11.5)

-- mickey
mick = gr.node('arc')
scene:add_child(mick)
mickey = gr.mesh( 'mickey', './Assets/mickey.obj' )
mick:add_child(mickey)
mickey:set_material(stone)
mick:scale(5.9, 5.9, 5.9)
mick:rotate('X', -90)
mick:rotate('Y', -90)
mick:translate(-10, 2, 10)


-- cow
cow = gr.mesh('cow', './Assets/cow.obj')
cow:set_material(cow_white)
cow:translate(0.0, 3.637, 0.0)
cow:scale(-1, 1, 1)
cow:translate(11, 0, 1.0)
scene:add_child(cow)

-- Place a ring of arches.

gate_number = 1
for _, pt2 in pairs({
   {{7,0,-7}, -45, {1,1 + math.random(),1}},
   {{0,0,-10}, 0, {1,1 + math.random(),1}},
   {{-7,0,-7}, 45, {1,1 + math.random(),1}},
   {{-10,0,0}, 90, {1,1 + math.random(),1}},
   {{-7,0,7}, -45, {1,1 + math.random(),1}},
   {{0,0,10}, 0, {1,1 + math.random(),1}}
   }) do
   an_gate = gr.node('gate' .. tostring(arc_number))
   an_gate:rotate('Y',  pt2[2])
   an_gate:scale(table.unpack(pt2[3]))
   scene:add_child(an_gate)
   an_gate:add_child(gate)
   an_gate:translate(table.unpack(pt2[1]))

   gate_number = gate_number + 1
end

gr.render(scene,
	  'sample.png', 500, 500,
	  {0, 2, 50}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})})