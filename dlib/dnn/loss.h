// Copyright (C) 2015  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_DNn_LOSS_H_
#define DLIB_DNn_LOSS_H_

#include "loss_abstract.h"
#include "core.h"
#include "../matrix.h"

namespace dlib
{

// ----------------------------------------------------------------------------------------

    class loss_binary_hinge_ 
    {
    public:

        const static unsigned int sample_expansion_factor = 1;
        typedef float label_type;

        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const SUB_TYPE& sub,
            label_iterator iter
        ) const
        {
            const tensor& output_tensor = sub.get_output();
            DLIB_CASSERT(output_tensor.nr() == 1 && 
                         output_tensor.nc() == 1 && 
                         output_tensor.k() == 1,"");
            DLIB_CASSERT(output_tensor.num_samples()%sample_expansion_factor == 0,"");

            const float* out_data = output_tensor.host();
            for (unsigned long i = 0; i < output_tensor.num_samples(); ++i)
            {
                *iter++ = out_data[i];
            }
        }

        template <
            typename const_label_iterator,
            typename SUB_NET
            >
        double compute_loss (
            const tensor& input_tensor,
            const_label_iterator truth, 
            SUB_NET& sub
        ) const
        {
            const tensor& output_tensor = sub.get_output();
            tensor& grad = sub.get_gradient_input();

            DLIB_CASSERT(input_tensor.num_samples() != 0,"");
            DLIB_CASSERT(input_tensor.num_samples()%sample_expansion_factor == 0,"");
            DLIB_CASSERT(input_tensor.num_samples() == grad.num_samples(),"");
            DLIB_CASSERT(input_tensor.num_samples() == output_tensor.num_samples(),"");
            DLIB_CASSERT(output_tensor.nr() == 1 && 
                         output_tensor.nc() == 1 && 
                         output_tensor.k() == 1,"");

            // The loss we output is the average loss over the mini-batch.
            const double scale = 1.0/output_tensor.num_samples();
            double loss = 0;
            const float* out_data = output_tensor.host();
            float* g = grad.host();
            for (unsigned long i = 0; i < output_tensor.num_samples(); ++i)
            {
                const float y = *truth++;
                DLIB_CASSERT(y == +1 || y == -1, "y: " << y);
                const float temp = 1-y*out_data[i];
                if (temp > 0)
                {
                    loss += scale*temp;
                    g[i] += -scale*y;
                }
            }
            return loss;
        }

    };

    template <typename SUB_NET>
    using loss_binary_hinge = add_loss_layer<loss_binary_hinge_, SUB_NET>;

// ----------------------------------------------------------------------------------------

    class loss_no_label_ 
    {
    public:

        const static unsigned int sample_expansion_factor = 1;

        template <
            typename SUB_NET
            >
        double compute_loss (
            const tensor& input_tensor,
            SUB_NET& sub
        ) const
        {
            return 0;
        }

    };

    template <typename SUB_NET>
    using loss_no_label = add_loss_layer<loss_no_label_, SUB_NET>;

// ----------------------------------------------------------------------------------------

}

#endif // DLIB_DNn_LOSS_H_
