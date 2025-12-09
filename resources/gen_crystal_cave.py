#!/usr/bin/env python3
import math
import random
from pathlib import Path

# crystal mat names used for coloring the crystal meshes
CRYSTAL_MATERIALS = ["crystal_blue", "crystal_purple", "crystal_red"]

# global scale for every pos
SCALE = 10.0

# base floor height and wave settings
FLOOR_BASE_HEIGHT = -0.8
FLOOR_WAVE_ONE_AMPLITUDE = 0.2
FLOOR_WAVE_ONE_FREQUENCY = 1.5
FLOOR_WAVE_TWO_AMPLITUDE = 0.1
FLOOR_WAVE_TWO_X_FREQUENCY = 0.7
FLOOR_WAVE_TWO_Z_FREQUENCY = 2.1

# base vertical dist between floor and ceiling + waves for variation
CAVE_GAP_BASE = 1.6
CAVE_GAP_WAVE_ONE_AMPLITUDE = 0.25
CAVE_GAP_WAVE_ONE_X_FREQUENCY = 0.8
CAVE_GAP_WAVE_ONE_Z_FREQUENCY = 1.3
CAVE_GAP_WAVE_TWO_AMPLITUDE = 0.15
CAVE_GAP_WAVE_TWO_X_FREQUENCY = 1.7
CAVE_GAP_WAVE_TWO_Z_FREQUENCY = -0.5

# perlin noise table settings
NOISE_SEED_OFFSET = 1337
PERLIN_TABLE_SIZE = 256

# noise stronger farther from walls
EDGE_FALLOFF_dist = 0.35

# vert noise for floor and ceiling
FLOOR_NOISE_BASE_STRENGTH = 0.12
FLOOR_NOISE_EDGE_EXTRA_STRENGTH = 0.22
CEILING_NOISE_BASE_STRENGTH = 0.10
CEILING_NOISE_EDGE_EXTRA_STRENGTH = 0.18

# sideways noise (walls in and out)
pos_NOISE_BASE_STRENGTH = 0.05
pos_NOISE_EDGE_EXTRA_STRENGTH = 0.25

# how much cave tunnel stretched along forward dir
TUNNEL_Z_STRETCH_FACTOR = 1.5

cam_HEIGHT_ABOVE_FLOOR = 0.25

cam_LOOK_FORWARD_dist = 2.5
cam_LOOK_DOWN_OFFSET = 0.05

# tilt and shape settings for crystals / pillars
CRYSTAL_TILT_MAX_ANGLE = 0.2
CRYSTAL_TOP_HEIGHT_FRACTION = 0.7
CRYSTAL_INNER_RADIUS_SCALE = 0.45
CRYSTAL_HEIGHT_JITTER_FRACTION = 0.03

PILLAR_RADIUS_RANDOM_SCALE_MIN = 0.9
PILLAR_RADIUS_RANDOM_SCALE_MAX = 1.1
PILLAR_HEIGHT_JITTER_FRACTION = 0.03
PILLAR_bot_CAP_OFFSET_FRACTION = 0.01
PILLAR_TOP_CAP_OFFSET_FRACTION = 0.02
PILLAR_EXTRA_bot_DEPTH_FRACTION = 0.2


# smooth waves so the floor not perfectly flat
def floor_height(world_x, world_z):
    wave_1 = (
        FLOOR_WAVE_ONE_AMPLITUDE
        * math.sin(FLOOR_WAVE_ONE_FREQUENCY * world_x)
        * math.sin(FLOOR_WAVE_ONE_FREQUENCY * world_z)
    )
    wave_2 = FLOOR_WAVE_TWO_AMPLITUDE * math.sin(
        FLOOR_WAVE_TWO_X_FREQUENCY * world_x + FLOOR_WAVE_TWO_Z_FREQUENCY * world_z
    )
    return FLOOR_BASE_HEIGHT + wave_1 + wave_2


# smooth waves so ceiling not perfectly flat
def cave_gap(world_x, world_z):
    gap_wave_1 = CAVE_GAP_WAVE_ONE_AMPLITUDE * math.sin(
        CAVE_GAP_WAVE_ONE_X_FREQUENCY * world_x + CAVE_GAP_WAVE_ONE_Z_FREQUENCY * world_z
    )
    gap_wave_2 = CAVE_GAP_WAVE_TWO_AMPLITUDE * math.sin(
        CAVE_GAP_WAVE_TWO_X_FREQUENCY * world_x + CAVE_GAP_WAVE_TWO_Z_FREQUENCY * world_z
    )
    return CAVE_GAP_BASE + gap_wave_1 + gap_wave_2


# main scene building function (that creates cave .obj)
def generate_crystal_cave(
    obj_path="crystal_cave.obj",
    mtl_name="crystal_cave.mtl",
    seed=0,
    floor_size=4.0,
    floor_res=40,
    num_crystals=40,
):
    random.seed(seed)

    noise_random = random.Random(seed + NOISE_SEED_OFFSET)

    # perlin permutation table (shuffled list of numbers)
    perm_tab = list(range(PERLIN_TABLE_SIZE))
    noise_random.shuffle(perm_tab)
    perm_tab.extend(perm_tab)

    # fade curve for perlin smooth transitions
    def fade(noise_value):
        return noise_value * noise_value * noise_value * (
            noise_value * (noise_value * 6.0 - 15.0) + 10.0
        )

    # blends between two vals via weight between 0 & 1
    def lerp(start_value, end_value, blend_amount):
        return start_value + blend_amount * (end_value - start_value)

    # gradient function picks a diagonal dir and returns a signed dist along it
    def gradient(hash_value, offset_x, offset_y):
        gradient_idx = hash_value & 3
        if gradient_idx == 0:
            return offset_x + offset_y
        elif gradient_idx == 1:
            return -offset_x + offset_y
        elif gradient_idx == 2:
            return offset_x - offset_y
        else:
            return -offset_x - offset_y

    # 2D perlin noise for cave shell bumps
    def perlin2d(world_x, world_z):

        # integer grid cell corners around point
        cell_x_int = int(math.floor(world_x)) & 255
        cell_z_int = int(math.floor(world_z)) & 255

        # where we are inside the grid cell between 0 & 1
        cell_x_frac = world_x - math.floor(world_x)
        cell_z_frac = world_z - math.floor(world_z)

        # smooths fract vals so corners don't have sharp jumps
        smooth_x = fade(cell_x_frac)
        smooth_z = fade(cell_z_frac)

        # looks up four gradient hashes for corners of the grid cell
        hash_bot_left = perm_tab[perm_tab[cell_x_int] + cell_z_int]
        hash_top_left = perm_tab[perm_tab[cell_x_int] + cell_z_int + 1]
        hash_bot_right = perm_tab[perm_tab[cell_x_int + 1] + cell_z_int]
        hash_top_right = perm_tab[perm_tab[cell_x_int + 1] + cell_z_int + 1]

        # compute gradient values at all four corners
        bot_left_value = gradient(hash_bot_left, cell_x_frac, cell_z_frac)
        bot_right_value = gradient(hash_bot_right, cell_x_frac - 1.0, cell_z_frac)
        top_left_value = gradient(hash_top_left, cell_x_frac, cell_z_frac - 1.0)
        top_right_value = gradient(hash_top_right, cell_x_frac - 1.0, cell_z_frac - 1.0)

        # blend along x for the bot and top edges
        bot_edge_value = lerp(bot_left_value, bot_right_value, smooth_x)
        top_edge_value = lerp(top_left_value, top_right_value, smooth_x)

        # blend between bot and top to get the final value
        raw_noise_value = lerp(bot_edge_value, top_edge_value, smooth_z)

        # map from about minus one to plus one into zero to one
        return (raw_noise_value + 1.0) / 2.0

    # real mesh vertices in the scene
    verts = []

    # non mesh helper/non geo points (e.g. cam, light, etc.)
    ngeos = []

    # dict groups faces by mat name
    faces_by_mat = {}

    # adds a mesh vertex and return its 1-based idx (for .obj)
    def add_vertex(world_x, world_y, world_z):
        verts.append((world_x, world_y, world_z))
        return len(verts)

    # adds a nongeo point (e.g. cam or light) and returns its 1-based idx
    def add_helper_point(world_x, world_y, world_z):
        ngeos.append((world_x, world_y, world_z))
        return len(ngeos)

    # adds triangle face via three vertex idxs + mat it uses
    def add_face(vertex_idx_a, vertex_idx_b, vertex_idx_c, mat_name):
        faces_by_mat.setdefault(mat_name, []).append(
            (vertex_idx_a, vertex_idx_b, vertex_idx_c)
        )

    # builds the cave shell including floor ceiling and walls with noise
    def add_cave_shell():

        # grid of floor and ceiling vertex idxs
        floor_idx_grid = [
            [0] * (floor_res + 1) for _ in range(floor_res + 1)
        ]
        ceiling_idx_grid = [
            [0] * (floor_res + 1) for _ in range(floor_res + 1)
        ]

        # walks across square grid + computes noisy floor & ceiling pos at each pt
        for grid_x_idx in range(floor_res + 1):
            for grid_z_idx in range(floor_res + 1):

                # turns grid idx into a value between 0 & 1
                unit_x = grid_x_idx / floor_res
                unit_z = grid_z_idx / floor_res

                # bases regular grid in world space
                world_x = (unit_x - 0.5) * floor_size
                world_z = (unit_z - 0.5) * floor_size * TUNNEL_Z_STRETCH_FACTOR

                # smooths base floor and gap
                floor_y = floor_height(world_x, world_z)
                floor_to_ceiling_gap = cave_gap(world_x, world_z)

                # measures how close we are to edges
                dist_to_left_or_right_edge = min(unit_x, 1.0 - unit_x)
                dist_to_front_or_back_edge = min(unit_z, 1.0 - unit_z)
                dist_to_closest_edge = min(
                    dist_to_left_or_right_edge, dist_to_front_or_back_edge
                )

                # computes factor 0 in center and 1 near the walls
                edge_proximity_factor = max(
                    0.0,
                    min(
                        1.0,
                        (EDGE_FALLOFF_dist - dist_to_closest_edge)
                        / EDGE_FALLOFF_dist,
                    ),
                )

                # vert noise for floor and ceiling with stronger effect near the walls
                floor_noise_1 = perlin2d(world_x * 1.3, world_z * 1.3) - 0.5
                floor_noise_2 = perlin2d(
                    world_x * 3.1 + 7.3, world_z * 3.1 - 1.1
                ) - 0.5
                combined_floor_noise = (
                    0.7 * floor_noise_1 + 0.3 * floor_noise_2
                )

                ceiling_noise_1 = perlin2d(
                    world_x * 1.7 + 10.0, world_z * 1.7 - 4.0
                ) - 0.5
                ceiling_noise_2 = perlin2d(
                    world_x * 3.7 - 2.0, world_z * 3.7 + 8.0
                ) - 0.5
                combined_ceiling_noise = (
                    0.7 * ceiling_noise_1 + 0.3 * ceiling_noise_2
                )

                # computes vertical noise strength at this pt
                floor_noise_strength = FLOOR_NOISE_BASE_STRENGTH + (
                    FLOOR_NOISE_EDGE_EXTRA_STRENGTH * edge_proximity_factor
                )
                ceiling_noise_strength = CEILING_NOISE_BASE_STRENGTH + (
                    CEILING_NOISE_EDGE_EXTRA_STRENGTH * edge_proximity_factor
                )

                # floor and ceiling up or down based on noise
                floor_y += floor_noise_strength * combined_floor_noise
                ceiling_y = floor_y + floor_to_ceiling_gap + (
                    ceiling_noise_strength * combined_ceiling_noise
                )

                # positional noise in x and z to make walls even rougher
                pos_noise_x_1 = perlin2d(
                    world_x * 0.7 + 20.0, world_z * 0.7 - 10.0
                ) - 0.5
                pos_noise_z_1 = perlin2d(
                    world_x * 0.7 - 40.0, world_z * 0.7 + 5.0
                ) - 0.5
                pos_noise_x_2 = perlin2d(
                    world_x * 1.9 + 3.0, world_z * 1.9 + 11.0
                ) - 0.5
                pos_noise_z_2 = perlin2d(
                    world_x * 1.9 - 6.0, world_z * 1.9 - 9.0
                ) - 0.5

                combined_pos_noise_x = (
                    0.6 * pos_noise_x_1 + 0.4 * pos_noise_x_2
                )
                combined_pos_noise_z = (
                    0.6 * pos_noise_z_1 + 0.4 * pos_noise_z_2
                )

                # small nudges in center + stronger bulges near walls
                pos_noise_strength = pos_NOISE_BASE_STRENGTH + (
                    pos_NOISE_EDGE_EXTRA_STRENGTH * edge_proximity_factor
                )

                noisy_world_x = (
                    world_x + pos_noise_strength * combined_pos_noise_x
                )
                noisy_world_z = (
                    world_z + pos_noise_strength * combined_pos_noise_z
                )

                # store floor and ceiling vertex indices at this grid point
                floor_idx_grid[grid_x_idx][grid_z_idx] = add_vertex(
                    noisy_world_x, floor_y, noisy_world_z
                )
                ceiling_idx_grid[grid_x_idx][grid_z_idx] = add_vertex(
                    noisy_world_x, ceiling_y, noisy_world_z
                )

        # builds floor triangles over grid
        for grid_x_idx in range(floor_res):
            for grid_z_idx in range(floor_res):
                bot_left = floor_idx_grid[grid_x_idx][grid_z_idx]
                bot_right = floor_idx_grid[grid_x_idx + 1][grid_z_idx]
                top_left = floor_idx_grid[grid_x_idx][grid_z_idx + 1]
                top_right = floor_idx_grid[grid_x_idx + 1][grid_z_idx + 1]
                add_face(bot_left, bot_right, top_right, "rock")
                add_face(bot_left, top_right, top_left, "rock")

        # builds ceiling triangles over grid
        for grid_x_idx in range(floor_res):
            for grid_z_idx in range(floor_res):
                bot_left = ceiling_idx_grid[grid_x_idx][grid_z_idx]
                bot_right = ceiling_idx_grid[grid_x_idx + 1][grid_z_idx]
                top_left = ceiling_idx_grid[grid_x_idx][grid_z_idx + 1]
                top_right = ceiling_idx_grid[grid_x_idx + 1][grid_z_idx + 1]
                add_face(bot_left, top_right, bot_right, "rock")
                add_face(bot_left, top_left, top_right, "rock")

        # builds vertical wall strips along two sides at min and max x
        for grid_z_idx in range(floor_res):
            floor_left_bot = floor_idx_grid[0][grid_z_idx]
            floor_left_top = floor_idx_grid[0][grid_z_idx + 1]
            ceiling_left_bot = ceiling_idx_grid[0][grid_z_idx]
            ceiling_left_top = ceiling_idx_grid[0][grid_z_idx + 1]

            floor_right_bot = floor_idx_grid[floor_res][grid_z_idx]
            floor_right_top = floor_idx_grid[floor_res][grid_z_idx + 1]
            ceiling_right_bot = ceiling_idx_grid[floor_res][grid_z_idx]
            ceiling_right_top = ceiling_idx_grid[floor_res][grid_z_idx + 1]

            # left wall triangles
            add_face(floor_left_bot, floor_left_top, ceiling_left_top, "rock")
            add_face(floor_left_bot, ceiling_left_top, ceiling_left_bot, "rock")

            # right wall triangles
            add_face(floor_right_bot, ceiling_right_top, floor_right_top, "rock")
            add_face(floor_right_bot, ceiling_right_bot, ceiling_right_top, "rock")

        # builds vertical wall strips along near and far z edges
        for grid_x_idx in range(floor_res):
            floor_near_left = floor_idx_grid[grid_x_idx][0]
            floor_near_right = floor_idx_grid[grid_x_idx + 1][0]
            ceiling_near_left = ceiling_idx_grid[grid_x_idx][0]
            ceiling_near_right = ceiling_idx_grid[grid_x_idx + 1][0]

            floor_far_left = floor_idx_grid[grid_x_idx][floor_res]
            floor_far_right = floor_idx_grid[grid_x_idx + 1][floor_res]
            ceiling_far_left = ceiling_idx_grid[grid_x_idx][floor_res]
            ceiling_far_right = ceiling_idx_grid[grid_x_idx + 1][floor_res]

            # near wall triangles
            add_face(floor_near_left, ceiling_near_right, floor_near_right, "rock")
            add_face(floor_near_left, ceiling_near_left, ceiling_near_right, "rock")

            # far wall triangles
            add_face(floor_far_left, floor_far_right, ceiling_far_right, "rock")
            add_face(floor_far_left, ceiling_far_right, ceiling_far_left, "rock")

    # add a single upright crystal standing on the floor
    def add_crystal_at(center_x, center_z, base_radius, height, segments=6, mat_name="crystal_blue"):
        # find floor height at this point
        floor_y = floor_height(center_x, center_z)

        # store indices of the two rings that form the crystal sides
        base_ring_indices = []
        top_ring_indices = []

        # gives each crystal random rotation and slight tilt
        twist_angle = random.uniform(0.0, 2.0 * math.pi)
        tilt_angle = random.uniform(-CRYSTAL_TILT_MAX_ANGLE, CRYSTAL_TILT_MAX_ANGLE)
        tilt_dir_angle = random.uniform(0.0, 2.0 * math.pi)
        tilt_dir_x = math.cos(tilt_dir_angle)
        tilt_dir_z = math.sin(tilt_dir_angle)
        tilt_slope = math.tan(tilt_angle)

        # two rings of vertices that form sides of crystal
        for segment_idx in range(segments):
            angle_around = 2.0 * math.pi * segment_idx / segments + twist_angle
            cosine = math.cos(angle_around)
            sine = math.sin(angle_around)

            # base ring on the cave floor
            base_local_x = base_radius * cosine
            base_local_z = base_radius * sine
            base_world_y = floor_y
            base_world_x = center_x + base_local_x
            base_world_z = center_z + base_local_z
            base_ring_indices.append(add_vertex(base_world_x, base_world_y, base_world_z))

            # top ring tighter and pushed in tilt dir
            top_height_offset = CRYSTAL_TOP_HEIGHT_FRACTION * height
            top_local_x = CRYSTAL_INNER_RADIUS_SCALE * base_radius * cosine
            top_local_z = CRYSTAL_INNER_RADIUS_SCALE * base_radius * sine
            top_world_x = center_x + top_local_x + tilt_dir_x * top_height_offset * tilt_slope
            top_world_z = center_z + top_local_z + tilt_dir_z * top_height_offset * tilt_slope
            top_world_y = floor_y + top_height_offset
            top_ring_indices.append(add_vertex(top_world_x, top_world_y, top_world_z))

        # tip placed above and offset in tilt dir so crystal leans
        tip_world_x = center_x + tilt_dir_x * height * tilt_slope
        tip_world_z = center_z + tilt_dir_z * height * tilt_slope
        tip_world_y = floor_y + height
        tip_idx = add_vertex(tip_world_x, tip_world_y, tip_world_z)

        # connects base ring to top ring to form crystal sides
        for segment_idx in range(segments):
            base_current = base_ring_indices[segment_idx]
            base_next = base_ring_indices[(segment_idx + 1) % segments]
            top_current = top_ring_indices[segment_idx]
            top_next = top_ring_indices[(segment_idx + 1) % segments]
            add_face(base_current, base_next, top_next, mat_name)
            add_face(base_current, top_next, top_current, mat_name)

        # closes the base with triangles that fan out from the 1 base vertex
        for segment_idx in range(1, segments - 1):
            add_face(
                base_ring_indices[0],
                base_ring_indices[segment_idx],
                base_ring_indices[segment_idx + 1],
                mat_name,
            )

        # connects the top ring to the tip
        for segment_idx in range(segments):
            top_current = top_ring_indices[segment_idx]
            top_next = top_ring_indices[(segment_idx + 1) % segments]
            add_face(top_current, top_next, tip_idx, mat_name)

    # adds a crystal that hangs down from the ceiling
    def add_hanging_crystal_at(center_x, center_z, base_radius, height, segments=6, mat_name="crystal_blue"):
        # find floor and ceiling at this point
        floor_y = floor_height(center_x, center_z)
        floor_to_ceiling_gap = cave_gap(center_x, center_z)
        ceiling_y = floor_y + floor_to_ceiling_gap

        # stores indices of rings
        base_ring_indices = []
        mid_ring_indices = []

        # gives each crystal random rotation and slight tilt
        twist_angle = random.uniform(0.0, 2.0 * math.pi)
        tilt_angle = random.uniform(-CRYSTAL_TILT_MAX_ANGLE, CRYSTAL_TILT_MAX_ANGLE)
        tilt_dir_angle = random.uniform(0.0, 2.0 * math.pi)
        tilt_dir_x = math.cos(tilt_dir_angle)
        tilt_dir_z = math.sin(tilt_dir_angle)
        tilt_slope = math.tan(tilt_angle)

        # builds small base ring at ceiling and wider ring lower down
        for segment_idx in range(segments):
            angle_around = 2.0 * math.pi * segment_idx / segments + twist_angle
            cosine = math.cos(angle_around)
            sine = math.sin(angle_around)

            # base ring flush with ceiling
            base_world_y = ceiling_y
            base_height_relative_to_ceiling = 0.0
            base_local_x = 0.4 * base_radius * cosine
            base_local_z = 0.4 * base_radius * sine
            base_world_x = (
                center_x + base_local_x + tilt_dir_x * base_height_relative_to_ceiling * tilt_slope
            )
            base_world_z = (
                center_z + base_local_z + tilt_dir_z * base_height_relative_to_ceiling * tilt_slope
            )
            base_ring_indices.append(add_vertex(base_world_x, base_world_y, base_world_z))

            # mid ring lower and wider and shifted along tilt dir
            mid_world_y = ceiling_y - 0.4 * height
            mid_height_relative_to_ceiling = ceiling_y - mid_world_y
            mid_local_x = base_radius * cosine
            mid_local_z = base_radius * sine
            mid_world_x = (
                center_x + mid_local_x + tilt_dir_x * mid_height_relative_to_ceiling * tilt_slope
            )
            mid_world_z = (
                center_z + mid_local_z + tilt_dir_z * mid_height_relative_to_ceiling * tilt_slope
            )
            mid_ring_indices.append(add_vertex(mid_world_x, mid_world_y, mid_world_z))

        # tip extends further down + continues the same tilt
        tip_world_y = ceiling_y - height
        tip_height_relative_to_ceiling = ceiling_y - tip_world_y
        tip_world_x = center_x + tilt_dir_x * tip_height_relative_to_ceiling * tilt_slope
        tip_world_z = center_z + tilt_dir_z * tip_height_relative_to_ceiling * tilt_slope
        tip_idx = add_vertex(tip_world_x, tip_world_y, tip_world_z)

        # connects base ring to mid ring
        for segment_idx in range(segments):
            base_current = base_ring_indices[segment_idx]
            base_next = base_ring_indices[(segment_idx + 1) % segments]
            mid_current = mid_ring_indices[segment_idx]
            mid_next = mid_ring_indices[(segment_idx + 1) % segments]
            add_face(base_current, mid_next, base_next, mat_name)
            add_face(base_current, mid_current, mid_next, mat_name)

        # caps the base ring near the ceiling
        for segment_idx in range(1, segments - 1):
            add_face(
                base_ring_indices[0],
                base_ring_indices[segment_idx + 1],
                base_ring_indices[segment_idx],
                mat_name,
            )

        # connects mid ring to the tip
        for segment_idx in range(segments):
            mid_current = mid_ring_indices[segment_idx]
            mid_next = mid_ring_indices[(segment_idx + 1) % segments]
            add_face(mid_current, tip_idx, mid_next, mat_name)

    # adds rock pillar from floor to ceiling
    def add_cave_pillar_at(center_x, center_z, base_radius, segments=12, slices=10):

        # finds floor and ceiling at this point
        floor_y = floor_height(center_x, center_z)
        floor_to_ceiling_gap = cave_gap(center_x, center_z)

        # adjusts bot and top y so pillar blends into the shell
        bot_y = floor_y - PILLAR_EXTRA_bot_DEPTH_FRACTION * floor_to_ceiling_gap
        top_y = floor_y + 0.95 * floor_to_ceiling_gap
        pillar_height = top_y - bot_y

        # rings making up the pillar
        rings_of_indices = []

        # generates rings from bot to top with radius changing in roughly hourglass shape
        for slice_idx in range(slices + 1):
            height_fraction = slice_idx / slices
            ring_center_y = bot_y + height_fraction * pillar_height

            # radius larger near ends and smaller near middle
            radius_factor = 1.0 - 0.5 * math.cos(2.0 * math.pi * (height_fraction - 0.5))
            ring_radius = base_radius * radius_factor * random.uniform(
                PILLAR_RADIUS_RANDOM_SCALE_MIN, PILLAR_RADIUS_RANDOM_SCALE_MAX
            )

            current_ring_indices = []
            for segment_idx in range(segments):
                angle_around = 2.0 * math.pi * segment_idx / segments + random.uniform(-0.1, 0.1)
                cosine = math.cos(angle_around)
                sine = math.sin(angle_around)

                ring_world_x = center_x + ring_radius * cosine
                ring_world_z = center_z + ring_radius * sine
                ring_world_y = ring_center_y + random.uniform(
                    -PILLAR_HEIGHT_JITTER_FRACTION * pillar_height,
                    PILLAR_HEIGHT_JITTER_FRACTION * pillar_height,
                )

                current_ring_indices.append(add_vertex(ring_world_x, ring_world_y, ring_world_z))

            rings_of_indices.append(current_ring_indices)

        # bot cap so the pillar is closed + sits on floor
        bot_center_y = bot_y - PILLAR_bot_CAP_OFFSET_FRACTION * floor_to_ceiling_gap
        bot_center_idx = add_vertex(center_x, bot_center_y, center_z)

        # top cap just above top ring
        top_center_y = top_y + PILLAR_TOP_CAP_OFFSET_FRACTION * floor_to_ceiling_gap
        top_center_idx = add_vertex(center_x, top_center_y, center_z)

        # connects bot center to 1 ring
        bot_ring = rings_of_indices[0]
        for segment_idx in range(segments):
            bot_center = bot_center_idx
            ring_next = bot_ring[(segment_idx + 1) % segments]
            ring_current = bot_ring[segment_idx]
            add_face(bot_center, ring_next, ring_current, "rock")

        # connects consecutive rings to form pillar sides
        for slice_idx in range(len(rings_of_indices) - 1):
            lower_ring = rings_of_indices[slice_idx]
            upper_ring = rings_of_indices[slice_idx + 1]
            for segment_idx in range(segments):
                lower_current = lower_ring[segment_idx]
                lower_next = lower_ring[(segment_idx + 1) % segments]
                upper_current = upper_ring[segment_idx]
                upper_next = upper_ring[(segment_idx + 1) % segments]
                add_face(lower_current, lower_next, upper_next, "rock")
                add_face(lower_current, upper_next, upper_current, "rock")

        # connects last ring to top center
        top_ring = rings_of_indices[-1]
        for segment_idx in range(segments):
            ring_current = top_ring[segment_idx]
            ring_next = top_ring[(segment_idx + 1) % segments]
            add_face(ring_current, ring_next, top_center_idx, "rock")

    # adds all crystals and pillars into the cave
    def add_crystals_and_pillars():

        # side crystals
        for _ in range(num_crystals):
            side_choice = random.choice([-1, 1])
            if side_choice < 0:
                center_x = random.uniform(-0.45 * floor_size, -0.15 * floor_size)
            else:
                center_x = random.uniform(0.15 * floor_size, 0.45 * floor_size)
            center_z = random.uniform(-0.6 * floor_size, 0.7 * floor_size)
            base_radius = random.uniform(0.06, 0.18)
            height = random.uniform(0.5, 1.2)
            mat_name = random.choice(CRYSTAL_MATERIALS)
            add_crystal_at(center_x, center_z, base_radius, height, mat_name=mat_name)

        # center crystals
        for _ in range(num_crystals // 2):
            center_x = random.uniform(-0.1 * floor_size, 0.1 * floor_size)
            center_z = random.uniform(-0.6 * floor_size, 0.7 * floor_size)
            base_radius = random.uniform(0.04, 0.12)
            height = random.uniform(0.3, 0.8)
            mat_name = random.choice(CRYSTAL_MATERIALS)
            add_crystal_at(center_x, center_z, base_radius, height, mat_name=mat_name)

        # hanging crystals
        for _ in range(num_crystals):
            center_x = random.uniform(-0.2 * floor_size, 0.2 * floor_size)
            center_z = random.uniform(-0.5 * floor_size, 0.9 * floor_size)
            base_radius = random.uniform(0.04, 0.12)
            height = random.uniform(0.4, 0.9)
            mat_name = random.choice(CRYSTAL_MATERIALS)
            add_hanging_crystal_at(center_x, center_z, base_radius, height, mat_name=mat_name)

        # side pillars
        for _ in range(max(3, num_crystals // 6)):
            side_choice = random.choice([-1, 1])
            if side_choice < 0:
                center_x = random.uniform(-0.45 * floor_size, -0.25 * floor_size)
            else:
                center_x = random.uniform(0.25 * floor_size, 0.45 * floor_size)
            center_z = random.uniform(-0.6 * floor_size, 0.7 * floor_size)
            base_radius = random.uniform(0.10, 0.24)
            add_cave_pillar_at(center_x, center_z, base_radius)

        # center pillars
        for _ in range(max(1, num_crystals // 8)):
            center_x = random.uniform(-0.18 * floor_size, 0.18 * floor_size)
            center_z = random.uniform(-0.6 * floor_size, 0.7 * floor_size)
            base_radius = random.uniform(0.08, 0.16)
            add_cave_pillar_at(center_x, center_z, base_radius)

    cam_x = 0.0
    cam_z = -1.5
    cam_y = floor_height(cam_x, cam_z) + cam_HEIGHT_ABOVE_FLOOR

    look_target_x = 0.0
    look_target_z = cam_z + cam_LOOK_FORWARD_dist
    look_target_y = cam_y - cam_LOOK_DOWN_OFFSET

    cam_pos_idx = add_vertex(cam_x, cam_y, cam_z)
    look_target_idx = add_helper_point(look_target_x, look_target_y, look_target_z)
    up_vector_idx = add_helper_point(0.0, 1.0, 0.0)

    light_dir_idx = add_helper_point(-0.3, -0.8, -0.5)

    add_cave_shell()
    add_crystals_and_pillars()

    # writes out the obj file with vertices, faces, cam, and light
    obj_path = Path(obj_path)
    with obj_path.open("w") as output_file:

        # writes mat file reference
        output_file.write(f"mtllib {mtl_name}\n\n")

        # scales and writes all mesh vertices
        for world_x, world_y, world_z in verts:
            output_file.write(
                f"v {SCALE * world_x:.6f} {SCALE * world_y:.6f} {SCALE * world_z:.6f}\n"
            )
        output_file.write("\n")

        # scales and writes all non-geometry/helper points
        for world_x, world_y, world_z in ngeos:
            output_file.write(
                f"ng {SCALE * world_x:.6f} {SCALE * world_y:.6f} {SCALE * world_z:.6f}\n"
            )
        output_file.write("\n")

        # writes camera and light records
        output_file.write(
            f"camera {cam_pos_idx} {look_target_idx} {up_vector_idx} 35.0\n"
        )
        output_file.write(f"ld {light_dir_idx} 100 700 1.0\n\n")

        # writes all faces grouped by mat
        for mat_name, face_list in faces_by_mat.items():
            output_file.write(f"usemtl {mat_name}\n")
            for vertex_idx_a, vertex_idx_b, vertex_idx_c in face_list:
                output_file.write(f"f {vertex_idx_a} {vertex_idx_b} {vertex_idx_c}\n")
            output_file.write("\n")

    print(
        f"Wrote {obj_path} with {len(verts)} vertices and "
        f"{sum(len(face_list) for face_list in faces_by_mat.values())} faces."
    )

if __name__ == "__main__":
    generate_crystal_cave(floor_size=10.0, floor_res=80, num_crystals=200)