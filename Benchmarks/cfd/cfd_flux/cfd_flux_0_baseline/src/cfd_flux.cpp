#include"cfd_flux.h"

extern "C" {


inline void compute_velocity(float& density, float3& momentum, float3& velocity)
{
	velocity.x = momentum.x / density;
	velocity.y = momentum.y / density;
	velocity.z = momentum.z / density;
}

inline float compute_speed_sqd(float3& velocity)
{
	return velocity.x*velocity.x + velocity.y*velocity.y + velocity.z*velocity.z;
}

inline float compute_pressure(float& density, float& density_energy, float& speed_sqd)
{
	return (float(GAMMA) - float(1.0f))*(density_energy - float(0.5f)*density*speed_sqd);
}

inline float compute_speed_of_sound(float& density, float& pressure)
{
	return sqrt(float(GAMMA)*pressure / density);
}



void cfd_flux(float result[SIZE * NVAR], float elements_surrounding_elements [SIZE * NNB], float normals [SIZE * NNB * NDIM], float variables [SIZE * NVAR], float fc_momentum_x [SIZE * NDIM], float fc_momentum_y [SIZE * NDIM], float fc_momentum_z [SIZE * NDIM], float fc_density_energy [SIZE * NDIM])
{


    const float smoothing_coefficient = float(0.2f);

    float ff_variable[NVAR] = {1.399999976158142090, 1.680000066757202148, 0.000000000000000000, 0.000000000000000000, 3.507999897003173828};

    float3 ff_fc_momentum_x, ff_fc_momentum_y,ff_fc_momentum_z, ff_fc_density_energy;

    ff_fc_momentum_x.x = 3.016000270843505859; ff_fc_momentum_x.y = 0.000000000000000000; ff_fc_momentum_x.z = 0.000000000000000000; 
    ff_fc_momentum_y.x = 0.000000000000000000; ff_fc_momentum_y.y = 1.000000000000000000; ff_fc_momentum_y.z = 0.000000000000000000; 
    ff_fc_momentum_z.x = 0.000000000000000000; ff_fc_momentum_z.y = 0.000000000000000000; ff_fc_momentum_z.z = 1.000000000000000000; 
    ff_fc_density_energy.x = 5.409600257873535156; ff_fc_density_energy.y = 0.000000000000000000; ff_fc_density_energy.z = 0.000000000000000000;


    for(int i = 0; i < SIZE; i++)
    {
        int j, nb;
        float3 normal; float normal_len;
        float factor;

        float density_i = variables[NVAR*i + VAR_DENSITY];
        float3 momentum_i;
        momentum_i.x = variables[NVAR*i + (VAR_MOMENTUM+0)];
        momentum_i.y = variables[NVAR*i + (VAR_MOMENTUM+1)];
        momentum_i.z = variables[NVAR*i + (VAR_MOMENTUM+2)];
        float density_energy_i = variables[NVAR*i + VAR_DENSITY_ENERGY];

        float3 velocity_i;                           compute_velocity(density_i, momentum_i, velocity_i);
        float speed_sqd_i                          = compute_speed_sqd(velocity_i);
        float speed_i                              = sqrt(speed_sqd_i);
        float pressure_i                           = compute_pressure(density_i, density_energy_i, speed_sqd_i);
        float speed_of_sound_i                     = compute_speed_of_sound(density_i, pressure_i);
        float3 fc_i_momentum_x, fc_i_momentum_y, fc_i_momentum_z;
        float3 fc_i_density_energy;

        fc_i_momentum_x.x = fc_momentum_x[i*NDIM + 0];
        fc_i_momentum_x.y = fc_momentum_x[i*NDIM + 1];
        fc_i_momentum_x.z = fc_momentum_x[i*NDIM + 2];

        fc_i_momentum_y.x = fc_momentum_y[i*NDIM + 0];
        fc_i_momentum_y.y = fc_momentum_y[i*NDIM + 1];
        fc_i_momentum_y.z = fc_momentum_y[i*NDIM + 2];

        fc_i_momentum_z.x = fc_momentum_z[i*NDIM + 0];
        fc_i_momentum_z.y = fc_momentum_z[i*NDIM + 1];
        fc_i_momentum_z.z = fc_momentum_z[i*NDIM + 2];

        fc_i_density_energy.x = fc_density_energy[i*NDIM + 0];
        fc_i_density_energy.y = fc_density_energy[i*NDIM + 1];
        fc_i_density_energy.z = fc_density_energy[i*NDIM + 2];

        float flux_i_density = float(0.0f);
        float3 flux_i_momentum;
        flux_i_momentum.x = float(0.0f);
        flux_i_momentum.y = float(0.0f);
        flux_i_momentum.z = float(0.0f);
        float flux_i_density_energy = float(0.0f);

        float3 velocity_nb;
        float density_nb, density_energy_nb;
        float3 momentum_nb;
        float3 fc_nb_momentum_x, fc_nb_momentum_y, fc_nb_momentum_z;
        float3 fc_nb_density_energy;
        float speed_sqd_nb, speed_of_sound_nb, pressure_nb;

        for(j = 0; j < NNB; j++)
        {
            nb = elements_surrounding_elements[i*NNB + j];
            normal.x = normals[(i*NNB + j)*NDIM + 0];
            normal.y = normals[(i*NNB + j)*NDIM + 1];
            normal.z = normals[(i*NNB + j)*NDIM + 2];
            normal_len = sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);

            if(nb >= 0)
            {
                density_nb =        variables[nb*NVAR + VAR_DENSITY];
                momentum_nb.x =     variables[nb*NVAR + (VAR_MOMENTUM+0)];
                momentum_nb.y =     variables[nb*NVAR + (VAR_MOMENTUM+1)];
                momentum_nb.z =     variables[nb*NVAR + (VAR_MOMENTUM+2)];
                density_energy_nb = variables[nb*NVAR + VAR_DENSITY_ENERGY];
                                                    compute_velocity(density_nb, momentum_nb, velocity_nb);
                speed_sqd_nb                      = compute_speed_sqd(velocity_nb);
                pressure_nb                       = compute_pressure(density_nb, density_energy_nb, speed_sqd_nb);
                speed_of_sound_nb                 = compute_speed_of_sound(density_nb, pressure_nb);
                fc_nb_momentum_x.x = fc_momentum_x[nb*NDIM + 0];
                fc_nb_momentum_x.y = fc_momentum_x[nb*NDIM + 1];
                fc_nb_momentum_x.z = fc_momentum_x[nb*NDIM + 2];

                fc_nb_momentum_y.x = fc_momentum_y[nb*NDIM + 0];
                fc_nb_momentum_y.y = fc_momentum_y[nb*NDIM + 1];
                fc_nb_momentum_y.z = fc_momentum_y[nb*NDIM + 2];


                fc_nb_momentum_z.x = fc_momentum_z[nb*NDIM + 0];
                fc_nb_momentum_z.y = fc_momentum_z[nb*NDIM + 1];
                fc_nb_momentum_z.z = fc_momentum_z[nb*NDIM + 2];

                fc_nb_density_energy.x = fc_density_energy[nb*NDIM + 0];
                fc_nb_density_energy.y = fc_density_energy[nb*NDIM + 1];
                fc_nb_density_energy.z = fc_density_energy[nb*NDIM + 2];

                factor = -normal_len*smoothing_coefficient*float(0.5f)*(speed_i + sqrt(speed_sqd_nb) + speed_of_sound_i + speed_of_sound_nb);
                flux_i_density += factor*(density_i-density_nb);
                flux_i_density_energy += factor*(density_energy_i-density_energy_nb);
                flux_i_momentum.x += factor*(momentum_i.x-momentum_nb.x);
                flux_i_momentum.y += factor*(momentum_i.y-momentum_nb.y);
                flux_i_momentum.z += factor*(momentum_i.z-momentum_nb.z);

                factor = float(0.5f)*normal.x;
                flux_i_density += factor*(momentum_nb.x+momentum_i.x);
                flux_i_density_energy += factor*(fc_nb_density_energy.x+fc_i_density_energy.x);
                flux_i_momentum.x += factor*(fc_nb_momentum_x.x+fc_i_momentum_x.x);
                flux_i_momentum.y += factor*(fc_nb_momentum_y.x+fc_i_momentum_y.x);
                flux_i_momentum.z += factor*(fc_nb_momentum_z.x+fc_i_momentum_z.x);

                factor = float(0.5f)*normal.y;
                flux_i_density += factor*(momentum_nb.y+momentum_i.y);
                flux_i_density_energy += factor*(fc_nb_density_energy.y+fc_i_density_energy.y);
                flux_i_momentum.x += factor*(fc_nb_momentum_x.y+fc_i_momentum_x.y);
                flux_i_momentum.y += factor*(fc_nb_momentum_y.y+fc_i_momentum_y.y);
                flux_i_momentum.z += factor*(fc_nb_momentum_z.y+fc_i_momentum_z.y);

                factor = float(0.5f)*normal.z;
                flux_i_density += factor*(momentum_nb.z+momentum_i.z);
                flux_i_density_energy += factor*(fc_nb_density_energy.z+fc_i_density_energy.z);
                flux_i_momentum.x += factor*(fc_nb_momentum_x.z+fc_i_momentum_x.z);
                flux_i_momentum.y += factor*(fc_nb_momentum_y.z+fc_i_momentum_y.z);
                flux_i_momentum.z += factor*(fc_nb_momentum_z.z+fc_i_momentum_z.z);
            }
            else if(nb == -1)
            {
                flux_i_momentum.x += normal.x*pressure_i;
                flux_i_momentum.y += normal.y*pressure_i;
                flux_i_momentum.z += normal.z*pressure_i;
            }
            else if(nb == -2)
            {
                factor = float(0.5f)*normal.x;
                flux_i_density += factor*(ff_variable[VAR_MOMENTUM+0]+momentum_i.x);
                flux_i_density_energy += factor*(ff_fc_density_energy.x+fc_i_density_energy.x);
                flux_i_momentum.x += factor*(ff_fc_momentum_x.x + fc_i_momentum_x.x);
                flux_i_momentum.y += factor*(ff_fc_momentum_y.x + fc_i_momentum_y.x);
                flux_i_momentum.z += factor*(ff_fc_momentum_z.x + fc_i_momentum_z.x);

                factor = float(0.5f)*normal.y;
                flux_i_density += factor*(ff_variable[VAR_MOMENTUM+1]+momentum_i.y);
                flux_i_density_energy += factor*(ff_fc_density_energy.y+fc_i_density_energy.y);
                flux_i_momentum.x += factor*(ff_fc_momentum_x.y + fc_i_momentum_x.y);
                flux_i_momentum.y += factor*(ff_fc_momentum_y.y + fc_i_momentum_y.y);
                flux_i_momentum.z += factor*(ff_fc_momentum_z.y + fc_i_momentum_z.y);

                factor = float(0.5f)*normal.z;
                flux_i_density += factor*(ff_variable[VAR_MOMENTUM+2]+momentum_i.z);
                flux_i_density_energy += factor*(ff_fc_density_energy.z+fc_i_density_energy.z);
                flux_i_momentum.x += factor*(ff_fc_momentum_x.z + fc_i_momentum_x.z);
                flux_i_momentum.y += factor*(ff_fc_momentum_y.z + fc_i_momentum_y.z);
                flux_i_momentum.z += factor*(ff_fc_momentum_z.z + fc_i_momentum_z.z);

            }
        }

        result[i*NVAR + VAR_DENSITY] = flux_i_density;
        result[i*NVAR + (VAR_MOMENTUM+0)] = flux_i_momentum.x;
        result[i*NVAR + (VAR_MOMENTUM+1)] = flux_i_momentum.y;
        result[i*NVAR + (VAR_MOMENTUM+2)] = flux_i_momentum.z;
        result[i*NVAR + VAR_DENSITY_ENERGY] = flux_i_density_energy;
    }

    return;
}

void workload(float result[SIZE * NVAR], float elements_surrounding_elements [SIZE * NNB], float normals [SIZE * NNB * NDIM], 
                float variables [SIZE * NVAR], float fc_momentum_x [SIZE * NDIM], float fc_momentum_y [SIZE * NDIM], 
                float fc_momentum_z [SIZE * NDIM], float fc_density_energy [SIZE * NDIM])
{
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=result

    #pragma HLS INTERFACE m_axi port=elements_surrounding_elements offset=slave bundle=elements_surrounding_elements
    #pragma HLS INTERFACE m_axi port=normals offset=slave bundle=normals
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=variables
    #pragma HLS INTERFACE m_axi port=fc_momentum_x offset=slave bundle=fc_momentum_x
    #pragma HLS INTERFACE m_axi port=fc_momentum_y offset=slave bundle=fc_momentum_y
    #pragma HLS INTERFACE m_axi port=fc_momentum_z offset=slave bundle=fc_momentum_z
    #pragma HLS INTERFACE m_axi port=fc_density_energy offset=slave bundle=fc_density_energy
    
    #pragma HLS INTERFACE s_axilite port=result bundle=control

    #pragma HLS INTERFACE s_axilite port=elements_surrounding_elements bundle=control
    #pragma HLS INTERFACE s_axilite port=normals bundle=control
    #pragma HLS INTERFACE s_axilite port=variables bundle=control
    #pragma HLS INTERFACE s_axilite port=fc_momentum_x bundle=control
    #pragma HLS INTERFACE s_axilite port=fc_momentum_y bundle=control
    #pragma HLS INTERFACE s_axilite port=fc_momentum_z bundle=control
    #pragma HLS INTERFACE s_axilite port=fc_density_energy bundle=control

    #pragma HLS INTERFACE s_axilite port=return bundle=control


    cfd_flux(result, elements_surrounding_elements, normals, variables, fc_momentum_x, fc_momentum_y, fc_momentum_z, fc_density_energy);
    

	return;

}

}

