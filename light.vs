/*
 * File: light.vs
 * Project: CCSEP 2019 S2 Assignment
 * Created Date: 23/10/19 - 16:09:14
 * Author: Kevin Le - 19472960
 * Contact: kevin.le2@student.curtin.edu.au
 * -----
 * Purpose: Light Vertex Shader
 */

#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}