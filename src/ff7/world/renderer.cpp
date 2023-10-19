/****************************************************************************/
//    Copyright (C) 2009 Aali132                                            //
//    Copyright (C) 2018 quantumpencil                                      //
//    Copyright (C) 2018 Maxime Bacoux                                      //
//    Copyright (C) 2020 myst6re                                            //
//    Copyright (C) 2020 Chris Rizzitello                                   //
//    Copyright (C) 2020 John Pritchard                                     //
//    Copyright (C) 2023 Julian Xhokaxhiu                                   //
//    Copyright (C) 2023 Cosmos                                             //
//    Copyright (C) 2023 Tang-Tang Zhou                                     //
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

#include "renderer.h"
#include "camera.h"

#include "../../renderer.h"

namespace ff7::world {

    void init_load_wm_bot_blocks() {
        ff7_externals.world_init_load_wm_bot_block_7533AF();

        newRenderer.loadWorldMapExternalMesh();
        newRenderer.loadSnakeExternalMesh();
        newRenderer.loadCloudsExternalMesh();
        newRenderer.loadMeteorExternalMesh();
    }

    void destroy_graphics_objects() {
        ff7_externals.world_exit_destroy_graphics_objects_75A921();

        //newRenderer.unloadExternalMesh();
    }

    void wm0_overworld_draw_all() {
        auto game_object = (struct ff7_game_obj *)common_externals.get_game_object();
        update_view_matrix (game_object);

        if (game_object)
        {
            float f_offset = 0.003f;
            float n_offset = 0.0f;

            auto polygon_set = (ff7_polygon_set*)game_object->polygon_set_2EC;
            if(polygon_set)
            {
                {
                    auto matrix_set = polygon_set->matrix_set;

                    float a = matrix_set->matrix_projection->_33;
                    float b = matrix_set->matrix_projection->_34;


                    float f = b / (a + 1.0f) + f_offset;
                    float n = b / (a - 1.0f) + n_offset;

                    
                    matrix_set->matrix_projection->_33 = -(f + n) / (f -n);
                    matrix_set->matrix_projection->_34 = -(2*f*n) / (f - n);
                }
            }
        }

        newRenderer.drawWorldMapExternalMesh();        
        newRenderer.drawSnakeExternalMesh(); 

        ff7_externals.world_wm0_overworld_draw_all_74C179();
    }

    void wm0_overworld_draw_clouds()
    {
        newRenderer.drawCloudsExternalMesh();
    }

    void wm0_overworld_draw_meteor()
    {
        if (*ff7_externals.is_meteor_flag_on_E2AAE4) newRenderer.drawMeteorExternalMesh();
    }

    void wm2_underwater_draw_all() {
        ff7_externals.world_wm2_underwater_draw_all_74C3F0();

        auto game_object = (struct ff7_game_obj *)common_externals.get_game_object();
        update_view_matrix (game_object);
        newRenderer.drawWorldMapExternalMesh();
    }

    void wm3_snowstorm_draw_all() {
        ff7_externals.world_wm3_snowstorm_draw_all_74C589();

        auto game_object = (struct ff7_game_obj *)common_externals.get_game_object();
        update_view_matrix (game_object);
        newRenderer.drawWorldMapExternalMesh();
    }

    // This draw call is the first UI call that marks the start of the first UI draw section
    void wm0_draw_minimap_quad_graphics_object(ff7_graphics_object* quad_graphics_object, ff7_game_obj* game_object) {
        newRenderer.setTimeFilterEnabled(false);
        ff7_externals.engine_draw_graphics_object(quad_graphics_object, game_object);
    }

    // This draw call is the first call related to world effects. It marks the end of the first UI draw section
    void wm0_draw_world_effects_1_graphics_object(ff7_graphics_object* world_effects_1_graphics_object, ff7_game_obj* game_object) {
        newRenderer.setTimeFilterEnabled(true);
        ff7_externals.engine_draw_graphics_object(world_effects_1_graphics_object, game_object);
    }

    // This draw call is the UI call that marks the second UI draw section
    void wm0_draw_minimap_points_graphics_object(ff7_graphics_object* minimap_points_graphics_object, ff7_game_obj* game_object) {
        newRenderer.setTimeFilterEnabled(false);
        ff7_externals.engine_draw_graphics_object(minimap_points_graphics_object, game_object);
    }


    int get_camera_rotation_z()
    {
        return 0;
    }

    void world_copy_position_maybe(vector4<int> *a1)
    {
        if ( a1 )
            *a1 = *ff7_externals.world_player_pos_E04918;
    }

void world_draw_effects()
{
    short v0; // ax
    transform_matrix rot_matrix_0; // [esp+0h] [ebp-90h] BYREF
    transform_matrix rot_matrix_1; // [esp+20h] [ebp-70h] BYREF
    int v3; // [esp+40h] [ebp-50h]
    transform_matrix rot_matrix_2; // [esp+44h] [ebp-4Ch] BYREF
    int v5; // [esp+64h] [ebp-2Ch]
    int v6; // [esp+68h] [ebp-28h]
    vector3<short> delta_point; // [esp+6Ch] [ebp-24h] BYREF
    vector4<short> camera_direction; // [esp+74h] [ebp-1Ch] BYREF
    vector4<int> a1; // [esp+7Ch] [ebp-14h] BYREF
    world_effect_2d_list_node* eff_node; // [esp+8Ch] [ebp-4h]

    camera_direction.x = 0;
    camera_direction.y = -(short)ff7_externals.get_world_camera_front_rot_74D298();
    camera_direction.z = 0;
    ff7_externals.engine_apply_rotation_to_rot_matrix_662AD8((vector3<short>*)&camera_direction, &rot_matrix_1);
    camera_direction.x = -ff7_externals.world_get_world_current_camera_rotation_x_74D3C6();
    ff7_externals.engine_apply_rotation_to_rot_matrix_662AD8((vector3<short>*)&camera_direction, &rot_matrix_2);
    ff7_externals.world_copy_position_maybe_75042B(&a1);

    for (eff_node = ff7_externals.dword_E35648; eff_node; eff_node = eff_node->next)
    {
        v6 = eff_node->x - a1.x;
        v5 = eff_node->y - *ff7_externals.world_camera_delta_y_DE6A04;
        v3 = eff_node->z - a1.z;
        if ( v6 > -30000 && v6 < 30000 && v3 > -30000 && v3 < 30000 )
        {
        delta_point.x = v6;
        v0 = 0;//ff7_externals.world_sub_762F9A(v6, v3);
        delta_point.y = v5 - v0;
        delta_point.z = v3;
        if ( eff_node->apply_rotation_y )
        {
            camera_direction.z = 0;
            camera_direction.x = 0;
            camera_direction.y = eff_node->apply_rotation_y;
            ff7_externals.engine_apply_rotation_to_transform_matrix_6628DE((vector3<short>*)&camera_direction, (rotation_matrix*)&rot_matrix_0);
            ff7_externals.engine_set_game_engine_rot_matrix_663673((rotation_matrix*)&rot_matrix_0);
        }
        else
        {
            ff7_externals.engine_set_game_engine_rot_matrix_663673((rotation_matrix*)&rot_matrix_2);
        }
        ff7_externals.world_submit_draw_effects_75C283(
            &eff_node->texture_data,
            (int)&ff7_externals.byte_96D6A8[12 * eff_node->unknown_idx],
            &delta_point,
            eff_node->apply_rotation_y);
        }
    }
}

    int world_sub_762F9A(int x, int z)
    {
        int world_pos_x = ff7_externals.world_player_pos_E04918->x;
        int world_pos_y = ff7_externals.world_player_pos_E04918->y;
	    int world_pos_z = ff7_externals.world_player_pos_E04918->z;

        #define cplx vec2
        #define cplx_new(re, im) vec2(re, im)
        #define cplx_re(z) z.x
        #define cplx_im(z) z.y
        #define cplx_exp(z) (exp(z.x) * cplx_new(cos(z.y), sin(z.y)))
        #define cplx_scale(z, scalar) (z * scalar)
        #define cplx_abs(z) (sqrt(z.x * z.x + z.y * z.y))

        vector3<float> worldPos = {static_cast<float>(-x + world_pos_x), static_cast<float>(world_pos_y + 500), static_cast<float>(-z + world_pos_z)};
        struct matrix viewMatrix;
        ::memcpy(&viewMatrix.m[0][0], newRenderer.getViewMatrix(), sizeof(viewMatrix.m));

        vector3<float> viewPos = {0.0f, 0.0f, 0.0f};
        transform_point(&viewMatrix, &worldPos, &viewPos);

        float rp = -300000;

        vector2<float> planedir = { viewPos.x, viewPos.z };
        float planeDirLength = sqrtf(planedir.x * planedir.x + planedir.y * planedir.y);
        planedir.x /= planeDirLength;
        planedir.y /= planeDirLength;

        vector2<float> plane = {viewPos.y, sqrtf((viewPos.x) * (viewPos.x) + (viewPos.z) * (viewPos.z))};
        vector2<float> planeScaled = {plane.x / rp, plane.y / rp};
        vector2<float> planeScaledExp = {exp(planeScaled.x) * cos(planeScaled.y), planeScaled.x * sin(planeScaled.y)};
        vector2<float> circle = { rp * planeScaledExp.x - rp, rp * planeScaledExp.y};

        vector3<float> newViewSpacePos = {circle.y * planedir.x, circle.x, circle.y * planedir.y};
        struct matrix invViewMatrix;
        ::memcpy(&invViewMatrix.m[0][0], newRenderer.getInvViewMatrix(), sizeof(invViewMatrix.m));
        
        vector3<float> newWorldPos = {0.0f, 0.0f, 0.0f};
        transform_point(&invViewMatrix, &newViewSpacePos, &newWorldPos);

        return 0;//newWorldPos.y - world_pos_y;
    }

    void draw_shadow(ff7_graphics_object *, ff7_game_obj *)
    {
    }

    void engine_apply_4x4_matrix_product_between_matrices(matrix *a1, matrix *a2, matrix *a3)
    {
        struct game_mode* mode = getmode_cached();

        struct matrix projection_matrix;
        ::memcpy(&projection_matrix.m[0][0], &a2->m[0][0], sizeof(projection_matrix.m));    

        if (mode->driver_mode == MODE_WORLDMAP)
        {
            float f_offset = 0.003f;
            float n_offset = 0.0f;

            float a = projection_matrix._33;
            float b = projection_matrix._43;


            float f = b / (a + 1.0f) + f_offset;
            float n = b / (a - 1.0f) + n_offset;

            
            projection_matrix._33 = -(f + n) / (f -n);
            projection_matrix._43 = -(2*f*n) / (f - n);            
        }
    
        ff7_externals.engine_apply_4x4_matrix_product_between_matrices_66C6CD(a1, &projection_matrix, a3);
    }

    void world_submit_draw_meteor_and_clouds_7547A6(__int16 world_camera_front)
    {
        vector4<int> cloud_meteor_vertices[12]; // [esp+4h] [ebp-168h] BYREF
        transform_matrix v2; // [esp+C4h] [ebp-A8h] BYREF
        int v3; // [esp+E4h] [ebp-88h]
        int i; // [esp+E8h] [ebp-84h]
        int flag; // [esp+ECh] [ebp-80h]
        float v6; // [esp+F0h] [ebp-7Ch]
        vector4<short> cloud_meteor_points[12]; // [esp+F4h] [ebp-78h] BYREF
        int dummy; // [esp+158h] [ebp-14h] BYREF
        vector4<short> special_point; // [esp+15Ch] [ebp-10h] BYREF
        int v11; // [esp+164h] [ebp-8h]
        world_texture_data *clouds_graphics_data; // [esp+168h] [ebp-4h]

        flag = ff7_externals.sub_74C9A5();
        clouds_graphics_data = ff7_externals.clouds_graphics_data_array_E2A370;
        if ( ff7_externals.sub_74CBC2() )
        {
            v3 = ff7_externals.sub_74D4ED();
            if ( !v3 )
            v3 = 1;
            v6 = -180 - ff7::world::camera.getRotationOffsetX();//ff7_externals.sub_7540F6() - 44;// - (v3 >> 8);
            if ( world_camera_front - *ff7_externals.dword_E2BBA4 <= 2048 )
            {
            if ( *ff7_externals.dword_E2BBA4 - world_camera_front > 2048 )
                world_camera_front += 4096;
            }
            else
            {
            world_camera_front -= 4096;
            }
            v11 = v3 * (world_camera_front - *ff7_externals.dword_E2BBA4) / 10000;
            *ff7_externals.dword_E28F20 -= v11;
            if ( *ff7_externals.dword_E28F20 >= 0 )
            {
            if ( *ff7_externals.dword_E28F20 > 255 )
                *ff7_externals.dword_E28F20 -= 256;
            }
            else
            {
            *ff7_externals.dword_E28F20 += 256;
            }
            *ff7_externals.dword_E2BBA4 += 10000 * v11 / v3;
            *ff7_externals.dword_E2BBA4 &= 0xFFFu;
            cloud_meteor_points[1].x = -160;
            cloud_meteor_points[0].x = -160;
            cloud_meteor_points[3].x = *ff7_externals.dword_E28F20 - 352;
            cloud_meteor_points[2].x = *ff7_externals.dword_E28F20 - 352;
            cloud_meteor_points[5].x = *ff7_externals.dword_E28F20 - 96;
            cloud_meteor_points[4].x = *ff7_externals.dword_E28F20 - 96;
            cloud_meteor_points[7].x = 160;
            cloud_meteor_points[6].x = 160;
            cloud_meteor_points[6].y = v6 - 64;
            cloud_meteor_points[4].y = v6 - 64;
            cloud_meteor_points[2].y = v6 - 64;
            cloud_meteor_points[0].y = v6 - 64;
            cloud_meteor_points[7].y = v6;
            cloud_meteor_points[5].y = v6;
            cloud_meteor_points[3].y = v6;
            cloud_meteor_points[1].y = v6;
            cloud_meteor_points[9].x = (v3 - 500) * ((-1048576 * world_camera_front) >> 20) / 10000 - 72;
            cloud_meteor_points[8].x = cloud_meteor_points[9].x;
            cloud_meteor_points[11].x = cloud_meteor_points[9].x + 144;
            cloud_meteor_points[10].x = cloud_meteor_points[9].x + 144;
            cloud_meteor_points[10].y = v6 - 112;
            cloud_meteor_points[8].y = v6 - 112;
            cloud_meteor_points[11].y = v6 - 48;
            cloud_meteor_points[9].y = v6 - 48;
            special_point.y = 0;
            special_point.x = 0;
            special_point.z = ff7_externals.sub_74D319();
            ff7_externals.engine_apply_rotation_to_transform_matrix_6628DE((vector3<short>*)&special_point, (rotation_matrix*)&v2);
            ff7_externals.engine_set_game_engine_rot_matrix_663673((rotation_matrix*)&v2);
            v2.pos_x = *ff7_externals.world_viewport_x_E2C424 + 160;
            v2.pos_y = 40;
            ff7_externals.engine_set_game_engine_position_663707((rotation_matrix*)&v2);
            for ( i = 0; i < 12; ++i )
            ff7_externals.engine_apply_translation_with_delta_662ECC(
                (vector3<short>*)&cloud_meteor_points[i],
                (vector3<int> *)&cloud_meteor_vertices[i],
                &dummy);
            if ( *ff7_externals.dword_E28F20 >= 192 )
            {
            clouds_graphics_data->top_left_x = cloud_meteor_vertices[0].x;
            clouds_graphics_data->bottom_left_x = cloud_meteor_vertices[1].x;
            clouds_graphics_data->top_right_x = cloud_meteor_vertices[2].x;
            clouds_graphics_data->bottom_right_x = cloud_meteor_vertices[3].x;
            clouds_graphics_data->top_left_y = cloud_meteor_vertices[0].y;
            clouds_graphics_data->bottom_left_y = cloud_meteor_vertices[1].y;
            clouds_graphics_data->top_right_y = cloud_meteor_vertices[2].y;
            clouds_graphics_data->bottom_right_y = cloud_meteor_vertices[3].y;
            clouds_graphics_data->field_1C = -1 - *ff7_externals.dword_E28F20 - 64;
            clouds_graphics_data->start_u_multiplier = clouds_graphics_data->field_1C;
            clouds_graphics_data->field_24[0] = -1;
            clouds_graphics_data->end_u_multiplier = -1;
            //ff7_externals.world_submit_draw_cloud_754493(clouds_graphics_data++);
            clouds_graphics_data->top_left_x = cloud_meteor_vertices[2].x;
            clouds_graphics_data->bottom_left_x = cloud_meteor_vertices[3].x;
            clouds_graphics_data->top_right_x = cloud_meteor_vertices[4].x;
            clouds_graphics_data->bottom_right_x = cloud_meteor_vertices[5].x;
            clouds_graphics_data->top_left_y = cloud_meteor_vertices[2].y;
            clouds_graphics_data->bottom_left_y = cloud_meteor_vertices[3].y;
            clouds_graphics_data->top_right_y = cloud_meteor_vertices[4].y;
            clouds_graphics_data->bottom_right_y = cloud_meteor_vertices[5].y;
            clouds_graphics_data->field_1C = 0;
            clouds_graphics_data->start_u_multiplier = 0;
            }
            else
            {
            clouds_graphics_data->top_left_x = cloud_meteor_vertices[0].x;
            clouds_graphics_data->bottom_left_x = cloud_meteor_vertices[1].x;
            clouds_graphics_data->top_right_x = cloud_meteor_vertices[4].x;
            clouds_graphics_data->bottom_right_x = cloud_meteor_vertices[5].x;
            clouds_graphics_data->top_left_y = cloud_meteor_vertices[0].y;
            clouds_graphics_data->bottom_left_y = cloud_meteor_vertices[1].y;
            clouds_graphics_data->top_right_y = cloud_meteor_vertices[4].y;
            clouds_graphics_data->bottom_right_y = cloud_meteor_vertices[5].y;
            clouds_graphics_data->field_1C = -64 - *ff7_externals.dword_E28F20;
            clouds_graphics_data->start_u_multiplier = clouds_graphics_data->field_1C;
            }
            clouds_graphics_data->field_24[0] = -1;
            clouds_graphics_data->end_u_multiplier = -1;
            //ff7_externals.world_submit_draw_cloud_754493(clouds_graphics_data++);
            clouds_graphics_data->top_left_x = cloud_meteor_vertices[4].x;
            clouds_graphics_data->bottom_left_x = cloud_meteor_vertices[5].x;
            clouds_graphics_data->top_right_x = cloud_meteor_vertices[6].x;
            clouds_graphics_data->bottom_right_x = cloud_meteor_vertices[7].x;
            clouds_graphics_data->top_left_y = cloud_meteor_vertices[4].y;
            clouds_graphics_data->bottom_left_y = cloud_meteor_vertices[5].y;
            clouds_graphics_data->top_right_y = cloud_meteor_vertices[6].y;
            clouds_graphics_data->bottom_right_y = cloud_meteor_vertices[7].y;
            clouds_graphics_data->field_1C = 0;
            clouds_graphics_data->start_u_multiplier = 0;
            clouds_graphics_data->field_24[0] = -1 - *ff7_externals.dword_E28F20;
            clouds_graphics_data->end_u_multiplier = clouds_graphics_data->field_24[0];
            //ff7_externals.world_submit_draw_cloud_754493(clouds_graphics_data);
            if ( *ff7_externals.is_meteor_flag_on_E2AAE4
            && cloud_meteor_vertices[10].x > (int)*ff7_externals.world_viewport_x_E2C424
            && cloud_meteor_vertices[8].x < (int)(*ff7_externals.world_viewport_y_E2C428 + 320) )
            {
            ff7_externals.word_969B98[4] = cloud_meteor_vertices[8].x;
            ff7_externals.word_969B98[5] = cloud_meteor_vertices[8].y;
            ff7_externals.word_969B98[12] = cloud_meteor_vertices[9].x;
            ff7_externals.word_969B98[13] = cloud_meteor_vertices[9].y;
            ff7_externals.word_969B98[8] = cloud_meteor_vertices[10].x;
            ff7_externals.word_969B98[9] = cloud_meteor_vertices[10].y;
            ff7_externals.word_969B98[16] = cloud_meteor_vertices[11].x;
            ff7_externals.word_969B98[17] = cloud_meteor_vertices[11].y;
            //ff7_externals.world_submit_draw_cloud_754493((world_texture_data *)ff7_externals.word_969B98);
            }
        }
    }
}