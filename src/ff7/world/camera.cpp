/****************************************************************************/
//    Copyright (C) 2009 Aali132                                            //
//    Copyright (C) 2018 quantumpencil                                      //
//    Copyright (C) 2018 Maxime Bacoux                                      //
//    Copyright (C) 2020 myst6re                                            //
//    Copyright (C) 2020 Chris Rizzitello                                   //
//    Copyright (C) 2020 John Pritchard                                     //
//    Copyright (C) 2022 Julian Xhokaxhiu                                   //
//    Copyright (C) 2022 Cosmos                                             //
//    Copyright (C) 2022 Tang-Tang Zhou                                     //
//                                                                          //
//    This file is part of FFNx                                             //
//                                                                          //
//    FFNx is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU General Public License as published by  //
//    the Free Software Foundation, either version 3 of the License         //
//                                                                          //
//    FFNx is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of        //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
//    GNU General Public License for more details.                          //
/****************************************************************************/

#include "camera.h"
#include "world.h"
#include "utils.h"

#include "defs.h"
#include "../../patch.h"

namespace ff7::world
{
    WorldCamera camera;

    void update_world_camera_front(int current_key_input, int prev_key_input)
    {
        const int player_model_id = ff7_externals.world_get_player_model_id();
        int movement_multiplier = *ff7_externals.world_movement_multiplier_DFC480;
        int world_map_type = *ff7_externals.world_map_type_E045E8;
        int camera_view_type = *ff7_externals.world_camera_viewtype_DFC4B4;
        //int movement_multiplier = *ff7_externals.world_movement_multiplier_DFC480;

        //bool highwind_view_and_square_pressed = camera_view_type == HIGHWIND_VIEW && is_key_pressed(current_key_input, SQUARE);
        if (world_map_type == SNOWSTORM)
        {
            auto flag = current_key_input & ANY_DIRECTIONAL_KEY;
            int delta_camera_step = ff7_externals.world_snowstorm_get_camera_movement_758B12(flag,
                                    !is_key_released(current_key_input, prev_key_input, CIRCLE));
            *ff7_externals.world_camera_front_DFC484 += delta_camera_step;
        }
        else// if (!highwind_view_and_square_pressed)
        {
            /*
            int direction_key, camera_rotate_speed = 0;
            if (camera_view_type == FRONT_VIEW && world_map_type != UNDERWATER && is_key_pressed(current_key_input, DOWN))
                direction_key = RIGHT;
            else
                direction_key = LEFT;
            camera_rotate_speed = is_key_pressed(current_key_input, direction_key) + is_key_pressed(current_key_input, L1);
            if (camera_rotate_speed)
            {
                int divider = world_map_type == UNDERWATER && (current_key_input & L1) == 0;
                *ff7_externals.world_camera_front_DFC484 -= ((2 - is_key_pressed(current_key_input, UP | DOWN)) * camera_rotate_speed * 8 * movement_multiplier) >> divider;
            }

            if (camera_view_type == FRONT_VIEW && world_map_type != UNDERWATER && is_key_pressed(current_key_input, DOWN))
                direction_key = LEFT;
            else
                direction_key = RIGHT;
            camera_rotate_speed = is_key_pressed(current_key_input, direction_key) + is_key_pressed(current_key_input, R1);
            if (camera_rotate_speed)
            {
                int divider = world_map_type == UNDERWATER && (current_key_input & R1) == 0;
                *ff7_externals.world_camera_front_DFC484 += ((2 - is_key_pressed(current_key_input, UP | DOWN)) * camera_rotate_speed * 8 * movement_multiplier) >> divider;
            }
            */

            auto rotationSpeed = camera.getRotationSpeed();
            camera.localTargetRotation.x = std::min(std::max(camera.localTargetRotation.x - rotationSpeed.x, -175.0f), -95.0f);
            camera.localTargetRotation.y = std::remainder(camera.localTargetRotation.y - rotationSpeed.y, 360.0f);

            auto last_valid_player_direction = (*ff7_externals.world_event_current_entity_ptr_E39AD8)->direction;
            if(last_valid_player_direction > 2048)
                last_valid_player_direction -= 4096;
            if(last_valid_player_direction < -2048)
                last_valid_player_direction += 4096;
            float player_direction = 360.0f * static_cast<float>(4096 - last_valid_player_direction - 2048) / 4096;
            auto joyDir = ff7::world::world.GetJoystickDirection();
            if (camera_view_type == HIGHWIND_VIEW || world_map_type == UNDERWATER || player_model_id == TINY_BRONCO || player_model_id == BUGGY || player_model_id == SUBMARINE)
            {
                int current_key_input = ff7_externals.world_get_current_key_input_status();
                float playerDeltaMovement = static_cast<float>(*ff7_externals.world_special_delta_movement_DE6A18);
                if(camera.isResetCameraRotationRequested())
                {
                    camera.localTargetRotation.y = player_direction;
                    ff7::world::camera.requestResetCameraRotation(false);
                }
                else
                {
                    int current_key_input = ff7_externals.world_get_current_key_input_status(); 
                    float playerDeltaMovement = static_cast<float>(*ff7_externals.world_special_delta_movement_DE6A18);
                    float t = 0.025 * movement_multiplier;   

                    bool isMovementButtonPressed = is_key_pressed(current_key_input, CIRCLE | R1 | SQUARE | L1 );

                    if (abs(joyDir.x) == 0.0f && !isMovementButtonPressed) t = 0.0f;                   
                        
                    int movement_speed = get_player_movement_speed(player_model_id);
                    float speedCoeff = std::abs(playerDeltaMovement) / (movement_speed);
                    float rotCoeff = abs(joyDir.x) > 0.0;
                    t *= isMovementButtonPressed || player_model_id == BUGGY ? std::max(0.1f, speedCoeff) : abs(joyDir.x);
                    
                    float diff = player_direction - camera.localTargetRotation.y;
                    if (diff > 180) 
                        camera.localTargetRotation.y += 360;
                    else if (diff < - 180)  camera.localTargetRotation.y -= 360;
                    camera.localTargetRotation.y = (1.0f - t) * camera.localTargetRotation.y + t * player_direction;
                }
                camera.targetRotation.y = camera.localTargetRotation.y;
                camera.targetRotation.x = camera.localTargetRotation.x;                
            }
            else
            {
                if (camera.isResetCameraRotationRequested())
                {
                    camera.targetRotation.x = camera.localTargetRotation.x;
                    camera.localTargetRotation.y = player_direction;
                    camera.targetRotation.y = player_direction;
                    ff7::world::camera.requestResetCameraRotation(false);
                }
                else
                {
                    camera.targetRotation.x = camera.localTargetRotation.x;
                    camera.targetRotation.y = camera.localTargetRotation.y;
                }
            }

            *ff7_externals.world_camera_front_DFC484 = camera.targetRotation.y * 4096 / 360.0f;
        }
        /*
        if (*ff7_externals.world_camera_front_DFC484 >= 0)
        {
            if (*ff7_externals.world_camera_front_DFC484 >= 4096)
                *ff7_externals.world_camera_front_DFC484 -= 4096;
        }
        else
            *ff7_externals.world_camera_front_DFC484 = 4096;
        */
    }

    void update_world_camera_rotation_y()
    {
        int movement_multiplier = *ff7_externals.world_movement_multiplier_DFC480;
        int world_map_type = *ff7_externals.world_map_type_E045E8;
        int camera_view_type = *ff7_externals.world_camera_viewtype_DFC4B4;
        if (world_map_type != SNOWSTORM)
        {
            //float targetRotationY = *ff7_externals.world_camera_front_DFC484 * 360.0f / 4096.0f;

            int world_map_type = *ff7_externals.world_map_type_E045E8;
            
            float diff = camera.targetRotation.y - camera.rotationOffset.y;
            if (diff > 180) 
                camera.rotationOffset.y += 360;
            else if (diff < - 180)  camera.rotationOffset.y -= 360;

            const float t = 0.1f * movement_multiplier;
            camera.rotationOffset.y = (1.0f - t) * camera.rotationOffset.y + t * camera.targetRotation.y;

            *ff7_externals.world_camera_rotation_y_DFC474 = camera.rotationOffset.y * 4096.0f / 360.0f;
        }
        else
        {
            int camera_view_type = *ff7_externals.world_camera_viewtype_DFC4B4;
            int movement_multiplier = *ff7_externals.world_movement_multiplier_DFC480;

            if (camera_view_type != HIGHWIND_VIEW)
                *ff7_externals.world_camera_var2_DE6B4C = (*ff7_externals.world_camera_var1_DF542C + 3 * (*ff7_externals.world_camera_var2_DE6B4C)) >> 2;

            if (*ff7_externals.world_camera_rotation_y_DFC474 >= *ff7_externals.world_camera_front_DFC484 - 2048)
            {
                if (*ff7_externals.world_camera_rotation_y_DFC474 > *ff7_externals.world_camera_front_DFC484 + 2048)
                    *ff7_externals.world_camera_rotation_y_DFC474 -= 4096;
            }
            else
                *ff7_externals.world_camera_rotation_y_DFC474 += 4096;

            if (movement_multiplier == 1)
                *ff7_externals.world_camera_rotation_y_DFC474 = (*ff7_externals.world_camera_front_DFC484 + 31 * (*ff7_externals.world_camera_rotation_y_DFC474)) >> 5;
            else
                *ff7_externals.world_camera_rotation_y_DFC474 = (*ff7_externals.world_camera_front_DFC484 + 15 * (*ff7_externals.world_camera_rotation_y_DFC474)) >> 4;
        }
    }

    void update_world_camera(short world_camera_rotation_y)
    {
        int movement_multiplier = *ff7_externals.world_movement_multiplier_DFC480;
        //float rotationX = -ff7_externals.world_get_camera_rotation_x_74F916() * 360.0f / 4096.0f;
        ff7_externals.world_get_camera_rotation_x_74F916();
       
        const float t = 0.25f * movement_multiplier;
        camera.rotationOffset.x = (1.0f - t) * camera.rotationOffset.x + t * camera.targetRotation.x;

        int world_map_type = *ff7_externals.world_map_type_E045E8;
        if (world_map_type == SNOWSTORM) camera.targetRotation.x = -105;

        bx::Vec3 up = { 0, 1, 0 };
        bx::Vec3 right = { 1, 0, 0 };
        auto quaternionH = bx::fromAxisAngle(up, M_PI * camera.rotationOffset.y / 180.0f);
        auto quaternionV = bx::fromAxisAngle(right, M_PI * camera.rotationOffset.x / 180.0f);

        *ff7_externals.world_current_camera_rotation_x_DE7418 = 2048 + static_cast<short>(4096.0f * camera.rotationOffset.x / 360.0f);

        auto quaternion = bx::mul(quaternionV, quaternionH);
        quaternion = bx::normalize(quaternion);

        float rotMat[16];
        bx::mtxFromQuaternion(rotMat, quaternion);

        float candidateDist = 10000 - (camera.zoomOffset + camera.getZoomSpeed());

        float maxZoomDist = camera.getMaxZoomDist();
        camera.zoomOffset = std::min(maxZoomDist, std::max(camera.minZoomDist, camera.zoomOffset - camera.getZoomSpeed()));

        if (world_map_type == SNOWSTORM) camera.zoomOffset = 0.0;

        auto rot_matrix = ff7_externals.world_camera_direction_matrix_DFC448;
        rot_matrix->r3_sub_matrix[0][0] = rotMat[0] * 4096;
        rot_matrix->r3_sub_matrix[0][1] = rotMat[1] * 4096;
        rot_matrix->r3_sub_matrix[0][2] = rotMat[2] * 4096;

        rot_matrix->r3_sub_matrix[1][0] = rotMat[4] * 4096;
        rot_matrix->r3_sub_matrix[1][1] = rotMat[5] * 4096;
        rot_matrix->r3_sub_matrix[1][2] = rotMat[6] * 4096;

        rot_matrix->r3_sub_matrix[2][0] = rotMat[8] * 4096;
        rot_matrix->r3_sub_matrix[2][1] = rotMat[9] * 4096;
        rot_matrix->r3_sub_matrix[2][2] = rotMat[10] * 4096;

        auto translation_matrix = ff7_externals.world_camera_position_matrix_DE6A20;
        translation_matrix->position[0] = 0;
        translation_matrix->position[1] = 0;
        translation_matrix->position[2] = camera.zoomOffset;

        *ff7_externals.world_camera_delta_y_DE6A04 = ff7_externals.world_player_pos_E04918->y + 500;
    }

    void WorldCamera::setRotationSpeed(float rotX, float rotY, float rotZ)
    {
        rotationSpeed.x = 0.5f * rotX / common_frame_multiplier;
        rotationSpeed.y = 0.5f * rotY / common_frame_multiplier;
        rotationSpeed.z = 0.5f * rotZ / common_frame_multiplier;
    }

    void WorldCamera::setZoomSpeed(float speed)
    {
        zoomSpeed = 0.5f * speed / common_frame_multiplier;
    }

    void WorldCamera::reset()
    {
        targetRotation.x = 0.0f;
        targetRotation.y = 0.0f;
        zoomOffset = 0.0f;
    }

    float WorldCamera::getMaxZoomDist()
    {
        const int player_model_id = ff7_externals.world_get_player_model_id();
        switch(player_model_id)
        {
            case SUBMARINE:
                return 5000;
            default:
                return 40000;
        }
    }
}