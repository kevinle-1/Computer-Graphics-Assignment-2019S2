/*
 * File: light.fs
 * Project: CCSEP 2019 S2 Assignment
 * Created Date: 23/10/19 - 16:09:14
 * Author: Kevin Le - 19472960
 * Contact: kevin.le2@student.curtin.edu.au
 * -----
 * Purpose: Light Fragment Shader
 */

#version 330 core
out vec4 FragColor;

uniform float intensity;

void main()
{
    FragColor = vec4(1, 0.5, 0.0, 1.0)  * intensity; //Orange
}