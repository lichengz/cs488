rootnode = gr.node('root')

red = gr.material({1.0, 0.0, 0.4}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.4, 1.0, 0.4}, {0.1, 0.1, 0.1}, 10)
pink = gr.material({1.0, 0.6, 0.8}, {0.1, 0.1, 0.1}, 10)
brown = gr.material({0.36, 0.27, 0.06}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
lightBlue = gr.material({0.43, 0.78, 0.86}, {0.1, 0.1, 0.1}, 10)
gray = gr.material({0.41, 0.41, 0.41}, {0.1, 0.1, 0.1}, 10)
darkgray = gr.material({0.25, 0.29, 0.30}, {0.1, 0.1, 0.1}, 10)
lightgray = gr.material({0.74, 0.78, 0.78}, {0.1, 0.1, 0.1}, 10)



torso = gr.mesh('cube', 'torso')
rootnode:add_child(torso)
torso:scale(1.2, 1, 1);
torso:translate(0.0, 0.45, 0.0)
torso:set_material(red)


torso_bot = gr.mesh('cube', 'torso_bot')
torso:add_child(torso_bot)
torso_bot:rotate("z",-5)
torso_bot:scale(0.9, 1.3, 0.9)
torso_bot:translate(0.0, -0.6, 0.0)
torso_bot:set_material(lightgray)

hip = gr.mesh('cube', 'hip')
torso_bot:add_child(hip)
hip:rotate("z",-5)
hip:rotate("x",5)
hip:scale(1.1, 0.3, 1)
hip:translate(-0.05, -1.2, 0.0)
hip:set_material(black)

--shoulder
shoulder = gr.mesh('cube', 'shoulder')
torso:add_child(shoulder)
shoulder:scale(1, 0.5, 0.1);
shoulder:translate(0.0, 1.2, -0.3)
shoulder:set_material(lightgray)

shoulder_l = gr.mesh('cube', 'shoulder_l')
shoulder:add_child(shoulder_l)
shoulder_l:scale(0.1, 0.5, 0.7);
shoulder_l:translate(-0.5, 1.2, 0)
shoulder_l:set_material(lightgray)

shoulder_r = gr.mesh('cube', 'shoulder_r')
shoulder:add_child(shoulder_r)
shoulder_r:scale(0.1, 0.5, 0.7);
shoulder_r:translate(0.5, 1.2, 0)
shoulder_r:set_material(lightgray)

--neck and head
neckJoint = gr.joint('neckJoint', {0, 0, 3.14/8}, {0, 0, 3.14/8})
shoulder:add_child(neckJoint)
neckJoint:translate(0.0, 1, 0.0)

neck = gr.mesh('sphere', 'neck')
neckJoint:add_child(neck)
neck:scale(0.2, 0.3, 0.2)
neck:translate(0.0, 1, 0.0)
neck:set_material(black)

headJoint = gr.joint('headJoint', {-3.14/8, 0, 3.14/8}, {-3.14/4, 0, 3.14/4})
neck:add_child(headJoint)
headJoint:translate(0.0, 1.0, 0.2)

head = gr.mesh('cube', 'head')
headJoint:add_child(head)
head:scale(0.5, 0.8, 0.5);
head:translate(0.0, 1.6, 0.0)
head:set_material(red)

-- head -> face
face = gr.mesh('sphere', 'face')
head:add_child(face)
face:scale(0.3, 0.4, 0.1)
face:translate(0.0, 1.4, 0.3)
face:set_material(white)

-- face -> eyes
left_eye = gr.mesh('sphere', 'left_eye')
head:add_child(left_eye)
left_eye:scale(0.06, 0.06, 0.06)
left_eye:translate(-0.1, 1.45, 0.4)
left_eye:set_material(green)
right_eye = gr.mesh('sphere', 'right_eye')
head:add_child(right_eye)
right_eye:scale(0.06, 0.06, 0.06)
right_eye:translate(0.1, 1.45, 0.4)
right_eye:set_material(green)

-- face -> small_head
small_head_base = gr.mesh('sphere', 'small_head_base')
head:add_child(small_head_base)
small_head_base:scale(0.15, 0.2, 0.1)
small_head_base:translate(0.0, 1.7, 0.35)
small_head_base:set_material(black)

small_head = gr.mesh('sphere', 'mini_face')
head:add_child(small_head)
small_head:scale(0.12, 0.14, 0.1)
small_head:translate(0.0, 1.75, 0.4)
small_head:set_material(pink)

-- face -> small_head -> mini_eyes
left_mini_eye = gr.mesh('sphere', 'left_mini_eye')
head:add_child(left_mini_eye)
left_mini_eye:scale(0.02, 0.02, 0.02)
left_mini_eye:translate(-0.05, 1.75, 0.5)
left_mini_eye:set_material(green)
right_mini_eye = gr.mesh('sphere', 'right_mini_eye')
head:add_child(right_mini_eye)
right_mini_eye:scale(0.02, 0.02, 0.02)
right_mini_eye:translate(0.05, 1.75, 0.5)
right_mini_eye:set_material(green)

-- left hip

leftHipJoint = gr.joint('leftHipJoint', {-3.14/8, 0, 3.14/8}, {0 , 0, 0})
hip:add_child(leftHipJoint)
leftHipJoint:translate(-0.5, -1.3, 0.0)

leftThigh = gr.mesh('sphere', 'leftThigh')
leftHipJoint:add_child(leftThigh)
leftThigh:scale(0.4, 0.8, 0.4)
leftThigh:translate(-0.5, -2, 0.0)
leftThigh:set_material(red)

leftKnee = gr.mesh('cube', 'leftKnee')
leftThigh:add_child(leftKnee)
leftKnee:scale(0.1, 0.4, 0.3)
leftKnee:rotate("x", -30)
leftKnee:translate(-0.5, -2.6, 0.2)
leftKnee:set_material(black)

leftThighJoint = gr.joint('leftThighJoint', {-3.14/8, 0, 3.14/8}, {0 , 0, 0})
leftThigh:add_child(leftThighJoint)
leftThighJoint:translate(-0.5, -2.2, 0.0)

leftCalf = gr.mesh('cube', 'leftCalf')
leftThighJoint:add_child(leftCalf)
leftCalf:scale(0.3, 1.2, 0.3)
leftCalf:translate(-0.5, -3.3, 0.0)
leftCalf:set_material(red)

leftCalfJoint = gr.joint('leftCalfJoint', {-3.14/8, 0, 3.14/8}, {0 , 0, 0})
leftCalf:add_child(leftCalfJoint)
leftCalfJoint:translate(-0.5, -3.9, 0.0)

leftFoot = gr.mesh('cube', 'leftFoot')
leftCalfJoint:add_child(leftFoot)
leftFoot:scale(0.2, 0.15, 0.7)
leftFoot:translate(-0.5, -3.9, 0.2)
leftFoot:set_material(lightgray)

-- right hip

rightHipJoint = gr.joint('rightHipJoint', {-3.14/8, 0, 3.14/8}, {0 , 0, 0})
hip:add_child(rightHipJoint)
rightHipJoint:translate(0.5, -1.3, 0.0)

rightThigh = gr.mesh('sphere', 'rightThigh')
rightHipJoint:add_child(rightThigh)
rightThigh:scale(0.4, 0.8, 0.4)
rightThigh:translate(0.5, -2, 0.0)
rightThigh:set_material(red)

rightKnee = gr.mesh('cube', 'rightKnee')
rightThigh:add_child(rightKnee)
rightKnee:scale(0.1, 0.4, 0.3)
rightKnee:rotate("x", -30)
rightKnee:translate(0.5, -2.6, 0.2)
rightKnee:set_material(black)

rightThighJoint = gr.joint('rightThighJoint', {-3.14/8, 0, 3.14/8}, {0 , 0, 0})
rightThigh:add_child(rightThighJoint)
rightThighJoint:translate(0.5, -2.2, 0.0)

rightCalf = gr.mesh('cube', 'rightCalf')
rightThighJoint:add_child(rightCalf)
rightCalf:scale(0.3, 1.2, 0.3)
rightCalf:translate(0.5, -3.3, 0.0)
rightCalf:set_material(red)

rightCalfJoint = gr.joint('rightCalfJoint', {-3.14/8, 0, 3.14/8}, {0 , 0, 0})
rightCalf:add_child(rightCalfJoint)
rightCalfJoint:translate(0.5, -3.9, 0.0)

rightFoot = gr.mesh('cube', 'rightFoot')
rightCalfJoint:add_child(rightFoot)
rightFoot:scale(0.2, 0.15, 0.7)
rightFoot:translate(0.5, -3.9, 0.2)
rightFoot:set_material(lightgray)

-- left shoulder and upper arm
leftShoulderJoint = gr.joint('leftShoulderJoint', {-3.14, 0, 3.14/2}, {-3.14/2, 0, 3.14/2})
shoulder:add_child(leftShoulderJoint)
leftShoulderJoint:translate(-0.6, 0.8, 0.0)


leftUpArm = gr.mesh('sphere', 'leftUpArm')
leftShoulderJoint:add_child(leftUpArm)
leftUpArm:scale(0.3, 0.8, 0.3)
leftUpArm:rotate('z',-40.0)
leftUpArm:translate(-1.0, 0.3, 0.0)
leftUpArm:set_material(red)

leftUpArm_1 = gr.mesh('sphere', 'leftUpArm_1')
leftUpArm:add_child(leftUpArm_1)
leftUpArm_1:scale(0.3, 0.4, 0.3)
leftUpArm_1:rotate('z',-40.0)
leftUpArm_1:translate(-0.9, 0.6, 0.0)
leftUpArm_1:set_material(lightgray)


-- left forearm
leftUpArmJoint = gr.joint('leftUpArmJoint', {-3.14/(1.2), 0, 3.14/2}, {0 , 0, 0})
leftUpArm:add_child(leftUpArmJoint)
leftUpArmJoint:translate(-1.4, -0.3, 0.0)

leftForeArm = gr.mesh('cube', 'leftForeArm')
leftUpArmJoint:add_child(leftForeArm)
leftForeArm:scale(1, 0.2, 0.2)
leftForeArm:rotate('z', 90.0)
leftForeArm:translate(-1.4, -0.6, 0.0)
leftForeArm:set_material(red)

-- left hand
leftForeArmJoint = gr.joint('leftUpArmJoint', {-3.14/(1.2), 0, 3.14/2}, {0 , 0, 0})
leftForeArm:add_child(leftForeArmJoint)
leftForeArmJoint:translate(-1.4, -1.2, 0.0)

leftHand = gr.mesh('cube', 'leftHand')
leftForeArmJoint:add_child(leftHand)
leftHand:scale(0.2, 0.2, 0.2)
leftHand:rotate('z', 90.0)
leftHand:translate(-1.4, -1.2, 0.0)
leftHand:set_material(lightgray)

-- right shoulder and upper arm
rightShoulderJoint = gr.joint('rightShoulderJoint', {-3.14, 0, 3.14/2}, {-3.14/2, 0, 3.14/2})
shoulder:add_child(rightShoulderJoint)
rightShoulderJoint:translate(0.6, 0.8, 0.0)


rightUpArm = gr.mesh('sphere', 'leftUpArm')
rightShoulderJoint:add_child(rightUpArm)
rightUpArm:scale(0.3, 0.8, 0.3)
rightUpArm:rotate('z',40.0)
rightUpArm:translate(1.0, 0.3, 0.0)
rightUpArm:set_material(red)

rightUpArm_1 = gr.mesh('sphere', 'rightUpArm_1')
rightUpArm:add_child(rightUpArm_1)
rightUpArm_1:scale(0.3, 0.4, 0.3)
rightUpArm_1:rotate('z',40.0)
rightUpArm_1:translate(0.9, 0.6, 0.0)
rightUpArm_1:set_material(lightgray)


-- left forearm
rightUpArmJoint = gr.joint('rightUpArmJoint', {-3.14/(1.2), 0, 3.14/2}, {0 , 0, 0})
rightUpArm:add_child(rightUpArmJoint)
rightUpArmJoint:translate(1.4, -0.3, 0.0)

rightForeArm = gr.mesh('cube', 'leftForeArm')
rightUpArmJoint:add_child(rightForeArm)
rightForeArm:scale(1, 0.2, 0.2)
rightForeArm:rotate('z', 90.0)
rightForeArm:translate(1.4, -0.6, 0.0)
rightForeArm:set_material(red)

-- left hand
rightForeArmJoint = gr.joint('rightForeArmJoint', {-3.14/(1.2), 0, 3.14/2}, {0 , 0, 0})
rightForeArm:add_child(rightForeArmJoint)
rightForeArmJoint:translate(1.4, -1.2, 0.0)

rightHand = gr.mesh('cube', 'rightHand')
rightForeArmJoint:add_child(rightHand)
rightHand:scale(0.2, 0.2, 0.2)
rightHand:rotate('z', 90.0)
rightHand:translate(1.4, -1.2, 0.0)
rightHand:set_material(lightgray)



rootnode:translate(0.0, 0.0, -8.0)

return rootnode