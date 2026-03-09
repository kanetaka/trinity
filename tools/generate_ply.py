import numpy as np

def generate_sample_ply(filename):
    # A few synthetic splats (e.g., 3 colored spheres)
    num_splats = 3
    
    # 1. Properties
    # x,y,z (float32)
    positions = np.array([
        [-1.0, 0.0, 0.0],
        [1.0, 0.0, 0.0],
        [0.0, 1.0, 0.0]
    ], dtype=np.float32)
    
    # f_dc_0, f_dc_1, f_dc_2 (float32)
    # spherical harmonics DC term relates to color: RGB = (SH_DC + 0.28209) / 0.28209 ... approx C = SH_DC * 0.28209 + 0.5
    # Let's just put something that parses properly. Red, Green, Blue approx
    sh_colors = np.array([
        [2.0, -1.0, -1.0], # Red-ish
        [-1.0, 2.0, -1.0], # Green-ish
        [-1.0, -1.0, 2.0]  # Blue-ish
    ], dtype=np.float32)
    
    # opacity (float32)
    opacities = np.array([[100.0], [10.0], [10.0]], dtype=np.float32) # Sigmoid(10) -> 1.0
    
    # scale_0, scale_1, scale_2 (float32) -> log scale
    # Scale of zero -> exp(0) = 1.0 size
    scales = np.array([
        [-1.0, -1.0, -1.0],
        [-1.0, -1.0, -1.0],
        [-1.0, -1.0, -1.0]
    ], dtype=np.float32)
    
    # rot_0, rot_1, rot_2, rot_3 (float32) -> quaternion (w, x, y, z)
    rotations = np.array([
        [1.0, 0.0, 0.0, 0.0],
        [1.0, 0.0, 0.0, 0.0],
        [1.0, 0.0, 0.0, 0.0]
    ], dtype=np.float32)
    
    # rest of SH (45 floats)
    sh_rest = np.zeros((num_splats, 45), dtype=np.float32)
    
    # Write PLY
    with open(filename, 'wb') as f:
        # Header
        header = f"""ply
format binary_little_endian 1.0
element vertex {num_splats}
property float x
property float y
property float z
property float nx
property float ny
property float nz
property float f_dc_0
property float f_dc_1
property float f_dc_2
"""
        for i in range(45):
            header += f"property float f_rest_{i}\n"
            
        header += """property float opacity
property float scale_0
property float scale_1
property float scale_2
property float rot_0
property float rot_1
property float rot_2
property float rot_3
end_header
"""
        f.write(header.encode('utf-8'))
        
        # Data
        for i in range(num_splats):
            # Write exactly in the order specified by the header
            f.write(positions[i].tobytes())
            f.write(np.zeros(3, dtype=np.float32).tobytes()) # nx, ny, nz
            f.write(sh_colors[i].tobytes())
            f.write(sh_rest[i].tobytes())
            f.write(opacities[i].tobytes())
            f.write(scales[i].tobytes())
            f.write(rotations[i].tobytes())

if __name__ == '__main__':
    generate_sample_ply('d:/proj/trinity/assets/splat.ply')
    print("Generated splat.ply")
