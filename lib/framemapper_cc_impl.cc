/* -*- c++ -*- */
/* 
 * Copyright 2014 Ron Economos.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "framemapper_cc_impl.h"
#include <stdio.h>

namespace gr {
  namespace dvbt2 {

    framemapper_cc::sptr
    framemapper_cc::make(dvbt2_framesize_t framesize, dvbt2_code_rate_t rate, dvbt2_constellation_t constellation, dvbt2_rotation_t rotation, int fecblocks, int tiblocks, dvbt2_extended_carrier_t carriermode, dvbt2_fftsize_t fftsize1, dvbt2_fftsize_t fftsize2, dvbt2_guardinterval_t guardinterval, dvbt2_l1constellation_t l1constellation, dvbt2_pilotpattern_t pilotpattern, int t2frames, int numdatasyms, dvbt2_papr_t paprmode, dvbt2_version_t version, dvbt2_preamble_t preamble1, dvbt2_preamble_t preamble2, dvbt2_inputmode_t inputmode, dvbt2_reservedbiasbits_t reservedbiasbits, dvbt2_l1scrambled_t l1scrambled, dvbt2_inband_t inband)
    {
      return gnuradio::get_initial_sptr
        (new framemapper_cc_impl(framesize, rate, constellation, rotation, fecblocks, tiblocks, carriermode, fftsize1, fftsize2, guardinterval, l1constellation, pilotpattern, t2frames, numdatasyms, paprmode, version, preamble1, preamble2, inputmode, reservedbiasbits, l1scrambled, inband));
    }

    /*
     * The private constructor
     */
    framemapper_cc_impl::framemapper_cc_impl(dvbt2_framesize_t framesize, dvbt2_code_rate_t rate, dvbt2_constellation_t constellation, dvbt2_rotation_t rotation, int fecblocks, int tiblocks, dvbt2_extended_carrier_t carriermode, dvbt2_fftsize_t fftsize1, dvbt2_fftsize_t fftsize2, dvbt2_guardinterval_t guardinterval, dvbt2_l1constellation_t l1constellation, dvbt2_pilotpattern_t pilotpattern, int t2frames, int numdatasyms, dvbt2_papr_t paprmode, dvbt2_version_t version, dvbt2_preamble_t preamble1, dvbt2_preamble_t preamble2, dvbt2_inputmode_t inputmode, dvbt2_reservedbiasbits_t reservedbiasbits, dvbt2_l1scrambled_t l1scrambled, dvbt2_inband_t inband)
      : gr::block("framemapper_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
        L1Pre *l1preinit = &L1_Signalling[0].l1pre_data;
        L1Post *l1postinit = &L1_Signalling[0].l1post_data;
        double normalization;
        int N_punc_temp, N_post_temp;
        if (framesize == gr::dvbt2::FECFRAME_NORMAL)
        {
            switch (constellation)
            {
                case gr::dvbt2::MOD_QPSK:
                    cell_size = 32400;
                    break;
                case gr::dvbt2::MOD_16QAM:
                    cell_size = 16200;
                    break;
                case gr::dvbt2::MOD_64QAM:
                    cell_size = 10800;
                    break;
                case gr::dvbt2::MOD_256QAM:
                    cell_size = 8100;
                    break;
            }
        }
        else
        {
            switch (constellation)
            {
                case gr::dvbt2::MOD_QPSK:
                    cell_size = 8100;
                    break;
                case gr::dvbt2::MOD_16QAM:
                    cell_size = 4050;
                    break;
                case gr::dvbt2::MOD_64QAM:
                    cell_size = 2700;
                    break;
                case gr::dvbt2::MOD_256QAM:
                    cell_size = 2025;
                    break;
            }
        }
        fef_present = FALSE;    /* for testing only */
        fef_length = 134144;    /*  "     "     "   */
        fef_interval = 1;       /*  "     "     "   */
        l1preinit->type = gr::dvbt2::STREAMTYPE_TS;
        l1preinit->bwt_ext = carriermode;
        fft_size = fftsize1;
        if (version == gr::dvbt2::VERSION_111)
        {
            l1preinit->s1 = preamble1;
        }
        else
        {
            l1preinit->s1 = preamble2;
            if (preamble2 == gr::dvbt2::PREAMBLE_T2_LITE_SISO || preamble2 == gr::dvbt2::PREAMBLE_T2_LITE_MISO)
            {
                fft_size = fftsize2;
            }
        }
        l1preinit->s2 = fft_size & 0x7;
        l1preinit->l1_repetition_flag = FALSE;
        l1preinit->guard_interval = guardinterval;
        l1preinit->papr = paprmode;
        l1preinit->l1_mod = l1constellation;
        l1preinit->l1_cod = 0;
        l1preinit->l1_fec_type = 0;
        if (fef_present == FALSE)
        {
            l1preinit->l1_post_info_size = KSIG_POST - 32;
        }
        else
        {
            l1preinit->l1_post_info_size = KSIG_POST + 34 - 32;
        }
        l1preinit->pilot_pattern = pilotpattern;
        l1preinit->tx_id_availability = 0;
        l1preinit->cell_id = 0;
        l1preinit->network_id = 0x3085;
        l1preinit->t2_system_id = 0x8001;
        l1preinit->num_t2_frames = t2frames;
        l1preinit->num_data_symbols = numdatasyms;
        l1preinit->regen_flag = FALSE;
        l1preinit->l1_post_extension = FALSE;
        l1preinit->num_rf = 1;
        l1preinit->current_rf_index = 0;
        l1preinit->t2_version = version;
        if (version == gr::dvbt2::VERSION_131)
        {
            l1preinit->l1_post_scrambled = l1scrambled;
        }
        else
        {
            l1preinit->l1_post_scrambled = FALSE;
        }
        l1preinit->t2_base_lite = FALSE;
        if (reservedbiasbits == gr::dvbt2::RESERVED_ON && version == gr::dvbt2::VERSION_131)
        {
            l1preinit->reserved = 0xf;
        }
        else
        {
            l1preinit->reserved = 0x0;
        }

        l1postinit->sub_slices_per_frame = 1;
        l1postinit->num_plp = 1;
        l1postinit->num_aux = 0;
        l1postinit->aux_config_rfu = 0;
        l1postinit->rf_idx = 0;
        l1postinit->frequency = 729833333;
        l1postinit->plp_id = 0;
        l1postinit->plp_type = 1;
        l1postinit->plp_payload_type = 3;
        l1postinit->ff_flag = 0;
        l1postinit->first_rf_idx = 0;
        l1postinit->first_frame_idx = 0;
        if (fef_present == FALSE)
        {
            l1postinit->plp_group_id = 1;
        }
        else
        {
            l1postinit->plp_group_id = 0;
        }
        l1postinit->plp_cod = rate;
        l1postinit->plp_mod = constellation;
        l1postinit->plp_rotation = rotation;
        l1postinit->plp_fec_type = framesize;
        l1postinit->plp_num_blocks_max = fecblocks;
        l1postinit->frame_interval = 1;
        l1postinit->time_il_length = tiblocks;
        l1postinit->time_il_type = 0;
        l1postinit->in_band_a_flag = 0;
        if (inband == gr::dvbt2::INBAND_ON && version == gr::dvbt2::VERSION_131)
        {
            l1postinit->in_band_b_flag = 1;
        }
        else
        {
            l1postinit->in_band_b_flag = 0;
        }
        if (reservedbiasbits == gr::dvbt2::RESERVED_ON && version == gr::dvbt2::VERSION_131)
        {
            l1postinit->reserved_1 = 0x7ff;
        }
        else
        {
            l1postinit->reserved_1 = 0x0;
        }
        if (version == gr::dvbt2::VERSION_111)
        {
            l1postinit->plp_mode = 0;
        }
        else
        {
            l1postinit->plp_mode = inputmode + 1;
        }
        if (fef_present == FALSE)
        {
            l1postinit->static_flag = 0;
            l1postinit->static_padding_flag = 0;    /* fix */
        }
        else
        {
            l1postinit->static_flag = 1;
            l1postinit->static_padding_flag = 1;
        }
        l1postinit->fef_length_msb = 0;
        if (reservedbiasbits == gr::dvbt2::RESERVED_ON && version == gr::dvbt2::VERSION_131)
        {
            l1postinit->reserved_2 = 0x3fffffff;
        }
        else
        {
            l1postinit->reserved_2 = 0;
        }
        l1postinit->frame_idx = 0;
        l1postinit->sub_slice_interval = 0;
        l1postinit->type_2_start = 0;
        l1postinit->l1_change_counter = 0;
        l1postinit->start_rf_idx = 0;
        if (reservedbiasbits == gr::dvbt2::RESERVED_ON && version == gr::dvbt2::VERSION_131)
        {
            l1postinit->reserved_3 = 0xff;
        }
        else
        {
            l1postinit->reserved_3 = 0;
        }
        l1postinit->plp_id = 0;
        l1postinit->plp_start = 0;
        l1postinit->plp_num_blocks = fecblocks;
        if (reservedbiasbits == gr::dvbt2::RESERVED_ON && version == gr::dvbt2::VERSION_131)
        {
            l1postinit->reserved_4 = 0xff;
            l1postinit->reserved_5 = 0xff;
        }
        else
        {
            l1postinit->reserved_4 = 0;
            l1postinit->reserved_5 = 0;
        }

        bch_poly_build_tables();
        l1pre_ldpc_lookup_generate();
        m_bpsk[0].real() =  1.0;
        m_bpsk[0].imag() =  0.0;
        m_bpsk[1].real() =  -1.0;
        m_bpsk[1].imag() =  0.0;
        unmodulated[0].real() =  0.0;
        unmodulated[0].imag() =  0.0;

        l1post_ldpc_lookup_generate();
        switch (l1constellation)
        {
            case gr::dvbt2::L1_MOD_BPSK:
                eta_mod = 1;
                break;
            case gr::dvbt2::L1_MOD_QPSK:
                normalization = sqrt(2);
                m_qpsk[0].real() =  1.0 / normalization;
                m_qpsk[0].imag() =  1.0 / normalization;
                m_qpsk[1].real() =  1.0 / normalization;
                m_qpsk[1].imag() = -1.0 / normalization;
                m_qpsk[2].real() = -1.0 / normalization;
                m_qpsk[2].imag() =  1.0 / normalization;
                m_qpsk[3].real() = -1.0 / normalization;
                m_qpsk[3].imag() = -1.0 / normalization;
                eta_mod = 2;
                break;
            case gr::dvbt2::L1_MOD_16QAM:
                normalization = sqrt(10);
                m_16qam[0].real()  =  3.0 / normalization;
                m_16qam[0].imag()  =  3.0 / normalization;
                m_16qam[1].real()  =  3.0 / normalization;
                m_16qam[1].imag()  =  1.0 / normalization;
                m_16qam[2].real()  =  1.0 / normalization;
                m_16qam[2].imag()  =  3.0 / normalization;
                m_16qam[3].real()  =  1.0 / normalization;
                m_16qam[3].imag()  =  1.0 / normalization;
                m_16qam[4].real()  =  3.0 / normalization;
                m_16qam[4].imag()  = -3.0 / normalization;
                m_16qam[5].real()  =  3.0 / normalization;
                m_16qam[5].imag()  = -1.0 / normalization;
                m_16qam[6].real()  =  1.0 / normalization;
                m_16qam[6].imag()  = -3.0 / normalization;
                m_16qam[7].real()  =  1.0 / normalization;
                m_16qam[7].imag()  = -1.0 / normalization;
                m_16qam[8].real()  = -3.0 / normalization;
                m_16qam[8].imag()  =  3.0 / normalization;
                m_16qam[9].real()  = -3.0 / normalization;
                m_16qam[9].imag()  =  1.0 / normalization;
                m_16qam[10].real() = -1.0 / normalization;
                m_16qam[10].imag() =  3.0 / normalization;
                m_16qam[11].real() = -1.0 / normalization;
                m_16qam[11].imag() =  1.0 / normalization;
                m_16qam[12].real() = -3.0 / normalization;
                m_16qam[12].imag() = -3.0 / normalization;
                m_16qam[13].real() = -3.0 / normalization;
                m_16qam[13].imag() = -1.0 / normalization;
                m_16qam[14].real() = -1.0 / normalization;
                m_16qam[14].imag() = -3.0 / normalization;
                m_16qam[15].real() = -1.0 / normalization;
                m_16qam[15].imag() = -1.0 / normalization;
                eta_mod = 4;
                break;
            case gr::dvbt2::L1_MOD_64QAM:
                normalization = sqrt(42);
                m_64qam[0].real() =   7.0 / normalization;
                m_64qam[0].imag() =   7.0 / normalization;
                m_64qam[1].real() =   7.0 / normalization;
                m_64qam[1].imag() =   5.0 / normalization;
                m_64qam[2].real() =   5.0 / normalization;
                m_64qam[2].imag() =   7.0 / normalization;
                m_64qam[3].real() =   5.0 / normalization;
                m_64qam[3].imag() =   5.0 / normalization;
                m_64qam[4].real() =   7.0 / normalization;
                m_64qam[4].imag() =   1.0 / normalization;
                m_64qam[5].real() =   7.0 / normalization;
                m_64qam[5].imag() =   3.0 / normalization;
                m_64qam[6].real() =   5.0 / normalization;
                m_64qam[6].imag() =   1.0 / normalization;
                m_64qam[7].real() =   5.0 / normalization;
                m_64qam[7].imag() =   3.0 / normalization;
                m_64qam[8].real() =   1.0 / normalization;
                m_64qam[8].imag() =   7.0 / normalization;
                m_64qam[9].real() =   1.0 / normalization;
                m_64qam[9].imag() =   5.0 / normalization;
                m_64qam[10].real() =  3.0 / normalization;
                m_64qam[10].imag() =  7.0 / normalization;
                m_64qam[11].real() =  3.0 / normalization;
                m_64qam[11].imag() =  5.0 / normalization;
                m_64qam[12].real() =  1.0 / normalization;
                m_64qam[12].imag() =  1.0 / normalization;
                m_64qam[13].real() =  1.0 / normalization;
                m_64qam[13].imag() =  3.0 / normalization;
                m_64qam[14].real() =  3.0 / normalization;
                m_64qam[14].imag() =  1.0 / normalization;
                m_64qam[15].real() =  3.0 / normalization;
                m_64qam[15].imag() =  3.0 / normalization;
                m_64qam[16].real() =  7.0 / normalization;
                m_64qam[16].imag() = -7.0 / normalization;
                m_64qam[17].real() =  7.0 / normalization;
                m_64qam[17].imag() = -5.0 / normalization;
                m_64qam[18].real() =  5.0 / normalization;
                m_64qam[18].imag() = -7.0 / normalization;
                m_64qam[19].real() =  5.0 / normalization;
                m_64qam[19].imag() = -5.0 / normalization;
                m_64qam[20].real() =  7.0 / normalization;
                m_64qam[20].imag() = -1.0 / normalization;
                m_64qam[21].real() =  7.0 / normalization;
                m_64qam[21].imag() = -3.0 / normalization;
                m_64qam[22].real() =  5.0 / normalization;
                m_64qam[22].imag() = -1.0 / normalization;
                m_64qam[23].real() =  5.0 / normalization;
                m_64qam[23].imag() = -3.0 / normalization;
                m_64qam[24].real() =  1.0 / normalization;
                m_64qam[24].imag() = -7.0 / normalization;
                m_64qam[25].real() =  1.0 / normalization;
                m_64qam[25].imag() = -5.0 / normalization;
                m_64qam[26].real() =  3.0 / normalization;
                m_64qam[26].imag() = -7.0 / normalization;
                m_64qam[27].real() =  3.0 / normalization;
                m_64qam[27].imag() = -5.0 / normalization;
                m_64qam[28].real() =  1.0 / normalization;
                m_64qam[28].imag() = -1.0 / normalization;
                m_64qam[29].real() =  1.0 / normalization;
                m_64qam[29].imag() = -3.0 / normalization;
                m_64qam[30].real() =  3.0 / normalization;
                m_64qam[30].imag() = -1.0 / normalization;
                m_64qam[31].real() =  3.0 / normalization;
                m_64qam[31].imag() = -3.0 / normalization;
                m_64qam[32].real() = -7.0 / normalization;
                m_64qam[32].imag() =  7.0 / normalization;
                m_64qam[33].real() = -7.0 / normalization;
                m_64qam[33].imag() =  5.0 / normalization;
                m_64qam[34].real() = -5.0 / normalization;
                m_64qam[34].imag() =  7.0 / normalization;
                m_64qam[35].real() = -5.0 / normalization;
                m_64qam[35].imag() =  5.0 / normalization;
                m_64qam[36].real() = -7.0 / normalization;
                m_64qam[36].imag() =  1.0 / normalization;
                m_64qam[37].real() = -7.0 / normalization;
                m_64qam[37].imag() =  3.0 / normalization;
                m_64qam[38].real() = -5.0 / normalization;
                m_64qam[38].imag() =  1.0 / normalization;
                m_64qam[39].real() = -5.0 / normalization;
                m_64qam[39].imag() =  3.0 / normalization;
                m_64qam[40].real() = -1.0 / normalization;
                m_64qam[40].imag() =  7.0 / normalization;
                m_64qam[41].real() = -1.0 / normalization;
                m_64qam[41].imag() =  5.0 / normalization;
                m_64qam[42].real() = -3.0 / normalization;
                m_64qam[42].imag() =  7.0 / normalization;
                m_64qam[43].real() = -3.0 / normalization;
                m_64qam[43].imag() =  5.0 / normalization;
                m_64qam[44].real() = -1.0 / normalization;
                m_64qam[44].imag() =  1.0 / normalization;
                m_64qam[45].real() = -1.0 / normalization;
                m_64qam[45].imag() =  3.0 / normalization;
                m_64qam[46].real() = -3.0 / normalization;
                m_64qam[46].imag() =  1.0 / normalization;
                m_64qam[47].real() = -3.0 / normalization;
                m_64qam[47].imag() =  3.0 / normalization;
                m_64qam[48].real() = -7.0 / normalization;
                m_64qam[48].imag() = -7.0 / normalization;
                m_64qam[49].real() = -7.0 / normalization;
                m_64qam[49].imag() = -5.0 / normalization;
                m_64qam[50].real() = -5.0 / normalization;
                m_64qam[50].imag() = -7.0 / normalization;
                m_64qam[51].real() = -5.0 / normalization;
                m_64qam[51].imag() = -5.0 / normalization;
                m_64qam[52].real() = -7.0 / normalization;
                m_64qam[52].imag() = -1.0 / normalization;
                m_64qam[53].real() = -7.0 / normalization;
                m_64qam[53].imag() = -3.0 / normalization;
                m_64qam[54].real() = -5.0 / normalization;
                m_64qam[54].imag() = -1.0 / normalization;
                m_64qam[55].real() = -5.0 / normalization;
                m_64qam[55].imag() = -3.0 / normalization;
                m_64qam[56].real() = -1.0 / normalization;
                m_64qam[56].imag() = -7.0 / normalization;
                m_64qam[57].real() = -1.0 / normalization;
                m_64qam[57].imag() = -5.0 / normalization;
                m_64qam[58].real() = -3.0 / normalization;
                m_64qam[58].imag() = -7.0 / normalization;
                m_64qam[59].real() = -3.0 / normalization;
                m_64qam[59].imag() = -5.0 / normalization;
                m_64qam[60].real() = -1.0 / normalization;
                m_64qam[60].imag() = -1.0 / normalization;
                m_64qam[61].real() = -1.0 / normalization;
                m_64qam[61].imag() = -3.0 / normalization;
                m_64qam[62].real() = -3.0 / normalization;
                m_64qam[62].imag() = -1.0 / normalization;
                m_64qam[63].real() = -3.0 / normalization;
                m_64qam[63].imag() = -3.0 / normalization;
                eta_mod = 6;
                break;
        }
        if ((preamble1 == gr::dvbt2::PREAMBLE_T2_SISO && version == gr::dvbt2::VERSION_111) || (preamble2 == gr::dvbt2::PREAMBLE_T2_SISO && version == gr::dvbt2::VERSION_131) || (preamble2 == gr::dvbt2::PREAMBLE_T2_LITE_SISO && version == gr::dvbt2::VERSION_131))
        {
            switch (fft_size)
            {
                case gr::dvbt2::FFTSIZE_1K:
                    N_P2 = 16;
                    C_P2 = 558;
                    break;
                case gr::dvbt2::FFTSIZE_2K:
                    N_P2 = 8;
                    C_P2 = 1118;
                    break;
                case gr::dvbt2::FFTSIZE_4K:
                    N_P2 = 4;
                    C_P2 = 2236;
                    break;
                case gr::dvbt2::FFTSIZE_8K:
                case gr::dvbt2::FFTSIZE_8K_T2GI:
                    N_P2 = 2;
                    C_P2 = 4472;
                    break;
                case gr::dvbt2::FFTSIZE_16K:
                case gr::dvbt2::FFTSIZE_16K_T2GI:
                    N_P2 = 1;
                    C_P2 = 8944;
                    break;
                case gr::dvbt2::FFTSIZE_32K:
                case gr::dvbt2::FFTSIZE_32K_T2GI:
                    N_P2 = 1;
                    C_P2 = 22432;
                    break;
            }
        }
        else
        {
            switch (fft_size)
            {
                case gr::dvbt2::FFTSIZE_1K:
                    N_P2 = 16;
                    C_P2 = 546;
                    break;
                case gr::dvbt2::FFTSIZE_2K:
                    N_P2 = 8;
                    C_P2 = 1098;
                    break;
                case gr::dvbt2::FFTSIZE_4K:
                    N_P2 = 4;
                    C_P2 = 2198;
                    break;
                case gr::dvbt2::FFTSIZE_8K:
                case gr::dvbt2::FFTSIZE_8K_T2GI:
                    N_P2 = 2;
                    C_P2 = 4398;
                    break;
                case gr::dvbt2::FFTSIZE_16K:
                case gr::dvbt2::FFTSIZE_16K_T2GI:
                    N_P2 = 1;
                    C_P2 = 8814;
                    break;
                case gr::dvbt2::FFTSIZE_32K:
                case gr::dvbt2::FFTSIZE_32K_T2GI:
                    N_P2 = 1;
                    C_P2 = 17612;
                    break;
            }
        }
        switch (fft_size)
        {
            case gr::dvbt2::FFTSIZE_1K:
                switch (pilotpattern)
                {
                    case gr::dvbt2::PILOT_PP1:
                        C_DATA = 764;
                        N_FC = 568;
                        C_FC = 402;
                        break;
                    case gr::dvbt2::PILOT_PP2:
                        C_DATA = 768;
                        N_FC = 710;
                        C_FC = 654;
                        break;
                    case gr::dvbt2::PILOT_PP3:
                        C_DATA = 798;
                        N_FC = 710;
                        C_FC = 490;
                        break;
                    case gr::dvbt2::PILOT_PP4:
                        C_DATA = 804;
                        N_FC = 780;
                        C_FC = 707;
                        break;
                    case gr::dvbt2::PILOT_PP5:
                        C_DATA = 818;
                        N_FC = 780;
                        C_FC = 544;
                        break;
                    case gr::dvbt2::PILOT_PP6:
                        C_DATA = 0;
                        N_FC = 0;
                        C_FC = 0;
                        break;
                    case gr::dvbt2::PILOT_PP7:
                        C_DATA = 0;
                        N_FC = 0;
                        C_FC = 0;
                        break;
                    case gr::dvbt2::PILOT_PP8:
                        C_DATA = 0;
                        N_FC = 0;
                        C_FC = 0;
                        break;
                }
                if (paprmode == gr::dvbt2::PAPR_TR || paprmode == gr::dvbt2::PAPR_BOTH)
                {
                    if (C_DATA != 0)
                    {
                        C_DATA -= 10;
                    }
                    if (N_FC != 0)
                    {
                        N_FC -= 10;
                    }
                    if (C_FC != 0)
                    {
                        C_FC -= 10;
                    }
                }
                break;
            case gr::dvbt2::FFTSIZE_2K:
                switch (pilotpattern)
                {
                    case gr::dvbt2::PILOT_PP1:
                        C_DATA = 1522;
                        N_FC = 1136;
                        C_FC = 804;
                        break;
                    case gr::dvbt2::PILOT_PP2:
                        C_DATA = 1532;
                        N_FC = 1420;
                        C_FC = 1309;
                        break;
                    case gr::dvbt2::PILOT_PP3:
                        C_DATA = 1596;
                        N_FC = 1420;
                        C_FC = 980;
                        break;
                    case gr::dvbt2::PILOT_PP4:
                        C_DATA = 1602;
                        N_FC = 1562;
                        C_FC = 1415;
                        break;
                    case gr::dvbt2::PILOT_PP5:
                        C_DATA = 1632;
                        N_FC = 1562;
                        C_FC = 1088;
                        break;
                    case gr::dvbt2::PILOT_PP6:
                        C_DATA = 0;
                        N_FC = 0;
                        C_FC = 0;
                        break;
                    case gr::dvbt2::PILOT_PP7:
                        C_DATA = 1646;
                        N_FC = 1632;
                        C_FC = 1396;
                        break;
                    case gr::dvbt2::PILOT_PP8:
                        C_DATA = 0;
                        N_FC = 0;
                        C_FC = 0;
                        break;
                }
                if (paprmode == gr::dvbt2::PAPR_TR || paprmode == gr::dvbt2::PAPR_BOTH)
                {
                    if (C_DATA != 0)
                    {
                        C_DATA -= 18;
                    }
                    if (N_FC != 0)
                    {
                        N_FC -= 18;
                    }
                    if (C_FC != 0)
                    {
                        C_FC -= 18;
                    }
                }
                break;
            case gr::dvbt2::FFTSIZE_4K:
                switch (pilotpattern)
                {
                    case gr::dvbt2::PILOT_PP1:
                        C_DATA = 3084;
                        N_FC = 2272;
                        C_FC = 1609;
                        break;
                    case gr::dvbt2::PILOT_PP2:
                        C_DATA = 3092;
                        N_FC = 2840;
                        C_FC = 2619;
                        break;
                    case gr::dvbt2::PILOT_PP3:
                        C_DATA = 3228;
                        N_FC = 2840;
                        C_FC = 1961;
                        break;
                    case gr::dvbt2::PILOT_PP4:
                        C_DATA = 3234;
                        N_FC = 3124;
                        C_FC = 2831;
                        break;
                    case gr::dvbt2::PILOT_PP5:
                        C_DATA = 3298;
                        N_FC = 3124;
                        C_FC = 2177;
                        break;
                    case gr::dvbt2::PILOT_PP6:
                        C_DATA = 0;
                        N_FC = 0;
                        C_FC = 0;
                        break;
                    case gr::dvbt2::PILOT_PP7:
                        C_DATA = 3328;
                        N_FC = 3266;
                        C_FC = 2792;
                        break;
                    case gr::dvbt2::PILOT_PP8:
                        C_DATA = 0;
                        N_FC = 0;
                        C_FC = 0;
                        break;
                }
                if (paprmode == gr::dvbt2::PAPR_TR || paprmode == gr::dvbt2::PAPR_BOTH)
                {
                    if (C_DATA != 0)
                    {
                        C_DATA -= 36;
                    }
                    if (N_FC != 0)
                    {
                        N_FC -= 36;
                    }
                    if (C_FC != 0)
                    {
                        C_FC -= 36;
                    }
                }
                break;
            case gr::dvbt2::FFTSIZE_8K:
            case gr::dvbt2::FFTSIZE_8K_T2GI:
                if (carriermode == gr::dvbt2::CARRIERS_NORMAL)
                {
                    switch (pilotpattern)
                    {
                        case gr::dvbt2::PILOT_PP1:
                            C_DATA = 6208;
                            N_FC = 4544;
                            C_FC = 3218;
                            break;
                        case gr::dvbt2::PILOT_PP2:
                            C_DATA = 6214;
                            N_FC = 5680;
                            C_FC = 5238;
                            break;
                        case gr::dvbt2::PILOT_PP3:
                            C_DATA = 6494;
                            N_FC = 5680;
                            C_FC = 3922;
                            break;
                        case gr::dvbt2::PILOT_PP4:
                            C_DATA = 6498;
                            N_FC = 6248;
                            C_FC = 5662;
                            break;
                        case gr::dvbt2::PILOT_PP5:
                            C_DATA = 6634;
                            N_FC = 6248;
                            C_FC = 4354;
                            break;
                        case gr::dvbt2::PILOT_PP6:
                            C_DATA = 0;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                        case gr::dvbt2::PILOT_PP7:
                            C_DATA = 6698;
                            N_FC = 6532;
                            C_FC = 5585;
                            break;
                        case gr::dvbt2::PILOT_PP8:
                            C_DATA = 6698;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                    }
                }
                else
                {
                    switch (pilotpattern)
                    {
                        case gr::dvbt2::PILOT_PP1:
                            C_DATA = 6296;
                            N_FC = 4608;
                            C_FC = 3264;
                            break;
                        case gr::dvbt2::PILOT_PP2:
                            C_DATA = 6298;
                            N_FC = 5760;
                            C_FC = 5312;
                            break;
                        case gr::dvbt2::PILOT_PP3:
                            C_DATA = 6584;
                            N_FC = 5760;
                            C_FC = 3978;
                            break;
                        case gr::dvbt2::PILOT_PP4:
                            C_DATA = 6588;
                            N_FC = 6336;
                            C_FC = 5742;
                            break;
                        case gr::dvbt2::PILOT_PP5:
                            C_DATA = 6728;
                            N_FC = 6336;
                            C_FC = 4416;
                            break;
                        case gr::dvbt2::PILOT_PP6:
                            C_DATA = 0;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                        case gr::dvbt2::PILOT_PP7:
                            C_DATA = 6788;
                            N_FC = 6624;
                            C_FC = 5664;
                            break;
                        case gr::dvbt2::PILOT_PP8:
                            C_DATA = 6788;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                    }
                }
                if (paprmode == gr::dvbt2::PAPR_TR || paprmode == gr::dvbt2::PAPR_BOTH)
                {
                    if (C_DATA != 0)
                    {
                        C_DATA -= 72;
                    }
                    if (N_FC != 0)
                    {
                        N_FC -= 72;
                    }
                    if (C_FC != 0)
                    {
                        C_FC -= 72;
                    }
                }
                break;
            case gr::dvbt2::FFTSIZE_16K:
            case gr::dvbt2::FFTSIZE_16K_T2GI:
                if (carriermode == gr::dvbt2::CARRIERS_NORMAL)
                {
                    switch (pilotpattern)
                    {
                        case gr::dvbt2::PILOT_PP1:
                            C_DATA = 12418;
                            N_FC = 9088;
                            C_FC = 6437;
                            break;
                        case gr::dvbt2::PILOT_PP2:
                            C_DATA = 12436;
                            N_FC = 11360;
                            C_FC = 10476;
                            break;
                        case gr::dvbt2::PILOT_PP3:
                            C_DATA = 12988;
                            N_FC = 11360;
                            C_FC = 7845;
                            break;
                        case gr::dvbt2::PILOT_PP4:
                            C_DATA = 13002;
                            N_FC = 12496;
                            C_FC = 11324;
                            break;
                        case gr::dvbt2::PILOT_PP5:
                            C_DATA = 13272;
                            N_FC = 12496;
                            C_FC = 8709;
                            break;
                        case gr::dvbt2::PILOT_PP6:
                            C_DATA = 13288;
                            N_FC = 13064;
                            C_FC = 11801;
                            break;
                        case gr::dvbt2::PILOT_PP7:
                            C_DATA = 13416;
                            N_FC = 13064;
                            C_FC = 11170;
                            break;
                        case gr::dvbt2::PILOT_PP8:
                            C_DATA = 13406;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                    }
                }
                else
                {
                    switch (pilotpattern)
                    {
                        case gr::dvbt2::PILOT_PP1:
                            C_DATA = 12678;
                            N_FC = 9280;
                            C_FC = 6573;
                            break;
                        case gr::dvbt2::PILOT_PP2:
                            C_DATA = 12698;
                            N_FC = 11600;
                            C_FC = 10697;
                            break;
                        case gr::dvbt2::PILOT_PP3:
                            C_DATA = 13262;
                            N_FC = 11600;
                            C_FC = 8011;
                            break;
                        case gr::dvbt2::PILOT_PP4:
                            C_DATA = 13276;
                            N_FC = 12760;
                            C_FC = 11563;
                            break;
                        case gr::dvbt2::PILOT_PP5:
                            C_DATA = 13552;
                            N_FC = 12760;
                            C_FC = 8893;
                            break;
                        case gr::dvbt2::PILOT_PP6:
                            C_DATA = 13568;
                            N_FC = 13340;
                            C_FC = 12051;
                            break;
                        case gr::dvbt2::PILOT_PP7:
                            C_DATA = 13698;
                            N_FC = 13340;
                            C_FC = 11406;
                            break;
                        case gr::dvbt2::PILOT_PP8:
                            C_DATA = 13688;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                    }
                }
                if (paprmode == gr::dvbt2::PAPR_TR || paprmode == gr::dvbt2::PAPR_BOTH)
                {
                    if (C_DATA != 0)
                    {
                        C_DATA -= 144;
                    }
                    if (N_FC != 0)
                    {
                        N_FC -= 144;
                    }
                    if (C_FC != 0)
                    {
                        C_FC -= 144;
                    }
                }
                break;
            case gr::dvbt2::FFTSIZE_32K:
            case gr::dvbt2::FFTSIZE_32K_T2GI:
                if (carriermode == gr::dvbt2::CARRIERS_NORMAL)
                {
                    switch (pilotpattern)
                    {
                        case gr::dvbt2::PILOT_PP1:
                            C_DATA = 0;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                        case gr::dvbt2::PILOT_PP2:
                            C_DATA = 24886;
                            N_FC = 22720;
                            C_FC = 20952;
                            break;
                        case gr::dvbt2::PILOT_PP3:
                            C_DATA = 0;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                        case gr::dvbt2::PILOT_PP4:
                            C_DATA = 26022;
                            N_FC = 24992;
                            C_FC = 22649;
                            break;
                        case gr::dvbt2::PILOT_PP5:
                            C_DATA = 0;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                        case gr::dvbt2::PILOT_PP6:
                            C_DATA = 26592;
                            N_FC = 26128;
                            C_FC = 23603;
                            break;
                        case gr::dvbt2::PILOT_PP7:
                            C_DATA = 26836;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                        case gr::dvbt2::PILOT_PP8:
                            C_DATA = 26812;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                    }
                }
                else
                {
                    switch (pilotpattern)
                    {
                        case gr::dvbt2::PILOT_PP1:
                            C_DATA = 0;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                        case gr::dvbt2::PILOT_PP2:
                            C_DATA = 25412;
                            N_FC = 23200;
                            C_FC = 21395;
                            break;
                        case gr::dvbt2::PILOT_PP3:
                            C_DATA = 0;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                        case gr::dvbt2::PILOT_PP4:
                            C_DATA = 26572;
                            N_FC = 25520;
                            C_FC = 23127;
                            break;
                        case gr::dvbt2::PILOT_PP5:
                            C_DATA = 0;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                        case gr::dvbt2::PILOT_PP6:
                            C_DATA = 27152;
                            N_FC = 26680;
                            C_FC = 24102;
                            break;
                        case gr::dvbt2::PILOT_PP7:
                            C_DATA = 27404;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                        case gr::dvbt2::PILOT_PP8:
                            C_DATA = 27376;
                            N_FC = 0;
                            C_FC = 0;
                            break;
                    }
                }
                if (paprmode == gr::dvbt2::PAPR_TR || paprmode == gr::dvbt2::PAPR_BOTH)
                {
                    if (C_DATA != 0)
                    {
                        C_DATA -= 288;
                    }
                    if (N_FC != 0)
                    {
                        N_FC -= 288;
                    }
                    if (C_FC != 0)
                    {
                        C_FC -= 288;
                    }
                }
                break;
        }
        if ((preamble1 == gr::dvbt2::PREAMBLE_T2_SISO && version == gr::dvbt2::VERSION_111) || (preamble2 == gr::dvbt2::PREAMBLE_T2_SISO && version == gr::dvbt2::VERSION_131) || (preamble2 == gr::dvbt2::PREAMBLE_T2_LITE_SISO && version == gr::dvbt2::VERSION_131))
        {
            if (guardinterval == gr::dvbt2::GI_1_128 && pilotpattern == gr::dvbt2::PILOT_PP7)
            {
                N_FC = 0;
                C_FC = 0;
            }
            if (guardinterval == gr::dvbt2::GI_1_32 && pilotpattern == gr::dvbt2::PILOT_PP4)
            {
                N_FC = 0;
                C_FC = 0;
            }
            if (guardinterval == gr::dvbt2::GI_1_16 && pilotpattern == gr::dvbt2::PILOT_PP2)
            {
                N_FC = 0;
                C_FC = 0;
            }
            if (guardinterval == gr::dvbt2::GI_19_256 && pilotpattern == gr::dvbt2::PILOT_PP2)
            {
                N_FC = 0;
                C_FC = 0;
            }
        }
        if (fef_present == FALSE)
        {
            N_punc_temp = (6 * (KBCH_1_2 - KSIG_POST)) / 5;
            N_post_temp = KSIG_POST + NBCH_PARITY + 9000 - N_punc_temp;
        }
        else
        {
            N_punc_temp = (6 * (KBCH_1_2 - (KSIG_POST + 34))) / 5;
            N_post_temp = (KSIG_POST + 34) + NBCH_PARITY + 9000 - N_punc_temp;
        }
        if (N_P2 == 1)
        {
            N_post = ceil((float)N_post_temp / (2 * (float)eta_mod)) * 2 * eta_mod;
        }
        else
        {
            N_post = ceil((float)N_post_temp / ((float)eta_mod * (float)N_P2)) * eta_mod * N_P2;
        }
        N_punc = N_punc_temp - (N_post - N_post_temp);
        l1preinit->l1_post_size = N_post / eta_mod;
        add_l1pre(&l1pre_cache[0]);
        l1_constellation = l1constellation;
        t2_frames = t2frames;
        t2_frame_num = 0;
        l1_scrambled = l1scrambled;
        stream_items = cell_size * fecblocks;
        if (N_FC == 0)
        {
            set_output_multiple((N_P2 * C_P2) + (numdatasyms * C_DATA));
            mapped_items = (N_P2 * C_P2) + (numdatasyms * C_DATA);
            if (mapped_items < (stream_items + 1840 + (N_post / eta_mod) + (N_FC - C_FC)))
            {
                fprintf(stderr, "Too many FEC blocks in T2 frame.\n");
                mapped_items = stream_items + 1840 + (N_post / eta_mod) + (N_FC - C_FC);    /* avoid segfault */
            }
            zigzag_interleave = (gr_complex *) malloc(sizeof(gr_complex) * mapped_items);
            if (zigzag_interleave == NULL) {
                fprintf(stderr, "Frame mapper 1st malloc, Out of memory.\n");
                exit(1);
            }
        }
        else
        {
            set_output_multiple((N_P2 * C_P2) + ((numdatasyms - 1) * C_DATA) + N_FC);
            mapped_items = (N_P2 * C_P2) + ((numdatasyms - 1) * C_DATA) + N_FC;
            if (mapped_items < (stream_items + 1840 + (N_post / eta_mod) + (N_FC - C_FC)))
            {
                fprintf(stderr, "Too many FEC blocks in T2 frame.\n");
                mapped_items = stream_items + 1840 + (N_post / eta_mod) + (N_FC - C_FC);    /* avoid segfault */
            }
            zigzag_interleave = (gr_complex *) malloc(sizeof(gr_complex) * mapped_items);
            if (zigzag_interleave == NULL) {
                fprintf(stderr, "Frame mapper 1st malloc, Out of memory.\n");
                exit(1);
            }
        }
        dummy_randomize = (gr_complex *) malloc(sizeof(gr_complex) * mapped_items - stream_items - 1840 - (N_post / eta_mod) - (N_FC - C_FC));
        if (dummy_randomize == NULL) {
            free(zigzag_interleave);
            fprintf(stderr, "Frame mapper 2nd malloc, Out of memory.\n");
            exit(1);
        }
        init_dummy_randomizer();
        init_l1_randomizer();
    }

    /*
     * Our virtual destructor.
     */
    framemapper_cc_impl::~framemapper_cc_impl()
    {
        free(dummy_randomize);
        free(zigzag_interleave);
    }

    void
    framemapper_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = stream_items * (noutput_items / mapped_items);
    }

#define CRC_POLY 0x04C11DB7

int framemapper_cc_impl::add_crc32_bits(unsigned char *in, int length)
{
    int crc = 0xffffffff;
    int b;
    int i = 0;

    for (int n = 0; n < length; n++)
    {
        b = in[i++] ^ ((crc >> 31) & 0x01);
        crc <<= 1;
        if (b) crc ^= CRC_POLY;
    }

    for (int n = 31; n >= 0; n--)
    {
        in[i++] = (crc & (1 << n)) ? 1 : 0;
    }
    return 32;
}

int framemapper_cc_impl::poly_mult(const int *ina, int lena, const int *inb, int lenb, int *out)
{
    memset(out, 0, sizeof(int) * (lena + lenb));

    for (int i = 0; i < lena; i++)
    {
        for (int j = 0; j < lenb; j++)
        {
            if (ina[i] * inb[j] > 0 ) out[i + j]++;    // count number of terms for this pwr of x
        }
    }
    int max = 0;
    for (int i = 0; i < lena + lenb; i++)
    {
        out[i] = out[i] & 1;    // If even ignore the term
        if(out[i]) max = i;
    }
    // return the size of array to house the result.
    return max + 1;

}

void framemapper_cc_impl::poly_pack(const int *pin, unsigned int* pout, int len)
{
    int lw = len / 32;
    int ptr = 0;
    unsigned int temp;
    if (len % 32) lw++;

    for (int i = 0; i < lw; i++)
    {
        temp = 0x80000000;
        pout[i] = 0;
        for (int j = 0; j < 32; j++)
        {
            if (pin[ptr++]) pout[i] |= temp;
            temp >>= 1;
        }
    }
}

void framemapper_cc_impl::bch_poly_build_tables(void)
{
    // Short polynomials
    const int polys01[]={1,1,0,1,0,1,0,0,0,0,0,0,0,0,1};
    const int polys02[]={1,0,0,0,0,0,1,0,1,0,0,1,0,0,1};
    const int polys03[]={1,1,1,0,0,0,1,0,0,1,1,0,0,0,1};
    const int polys04[]={1,0,0,0,1,0,0,1,1,0,1,0,1,0,1};
    const int polys05[]={1,0,1,0,1,0,1,0,1,1,0,1,0,1,1};
    const int polys06[]={1,0,0,1,0,0,0,1,1,1,0,0,0,1,1};
    const int polys07[]={1,0,1,0,0,1,1,1,0,0,1,1,0,1,1};
    const int polys08[]={1,0,0,0,0,1,0,0,1,1,1,1,0,0,1};
    const int polys09[]={1,1,1,1,0,0,0,0,0,1,1,0,0,0,1};
    const int polys10[]={1,0,0,1,0,0,1,0,0,1,0,1,1,0,1};
    const int polys11[]={1,0,0,0,1,0,0,0,0,0,0,1,1,0,1};
    const int polys12[]={1,1,1,1,0,1,1,1,1,0,1,0,0,1,1};

    int len;
    int polyout[2][200];

    len = poly_mult(polys01, 15, polys02,    15,  polyout[0]);
    len = poly_mult(polys03, 15, polyout[0], len, polyout[1]);
    len = poly_mult(polys04, 15, polyout[1], len, polyout[0]);
    len = poly_mult(polys05, 15, polyout[0], len, polyout[1]);
    len = poly_mult(polys06, 15, polyout[1], len, polyout[0]);
    len = poly_mult(polys07, 15, polyout[0], len, polyout[1]);
    len = poly_mult(polys08, 15, polyout[1], len, polyout[0]);
    len = poly_mult(polys09, 15, polyout[0], len, polyout[1]);
    len = poly_mult(polys10, 15, polyout[1], len, polyout[0]);
    len = poly_mult(polys11, 15, polyout[0], len, polyout[1]);
    len = poly_mult(polys12, 15, polyout[1], len, polyout[0]);
    poly_pack(polyout[0], m_poly_s_12, 168);
}

inline void framemapper_cc_impl::reg_6_shift(unsigned int *sr)
{
    sr[5] = (sr[5] >> 1) | (sr[4] << 31);
    sr[4] = (sr[4] >> 1) | (sr[3] << 31);
    sr[3] = (sr[3] >> 1) | (sr[2] << 31);
    sr[2] = (sr[2] >> 1) | (sr[1] << 31);
    sr[1] = (sr[1] >> 1) | (sr[0] << 31);
    sr[0] = (sr[0] >> 1);
}

void framemapper_cc_impl::l1pre_ldpc_lookup_generate(void)
{
    int im;
    int index;
    int pbits;
    int q;
    index = 0;
    im = 0;

    pbits = FRAME_SIZE_SHORT - NBCH_1_4;    //number of parity bits
    q = 36;

    for (int row = 0; row < 9; row++)
    {
        for(int n = 0; n < 360; n++)
        {
            for (int col = 1; col <= ldpc_tab_1_4S[row][0]; col++)
            {
                l1pre_ldpc_encode.p[index] = (ldpc_tab_1_4S[row][col] + (n * q)) % pbits;
                l1pre_ldpc_encode.d[index] = im;
                index++;
            }
            im++;
        }
    }
    l1pre_ldpc_encode.table_length = index;
}

void framemapper_cc_impl::l1post_ldpc_lookup_generate(void)
{
    int im;
    int index;
    int pbits;
    int q;
    index = 0;
    im = 0;

    pbits = FRAME_SIZE_SHORT - NBCH_1_2;    //number of parity bits
    q = 25;

    for (int row = 0; row < 20; row++)
    {
        for(int n = 0; n < 360; n++)
        {
            for (int col = 1; col <= ldpc_tab_1_2S[row][0]; col++)
            {
                l1post_ldpc_encode.p[index] = (ldpc_tab_1_2S[row][col] + (n * q)) % pbits;
                l1post_ldpc_encode.d[index] = im;
                index++;
            }
            im++;
        }
    }
    l1post_ldpc_encode.table_length = index;
}

void framemapper_cc_impl::add_l1pre(gr_complex *out)
{
    int temp, offset_bits = 0;
    unsigned char b, value;
    unsigned int shift[6];
    int plen = FRAME_SIZE_SHORT - NBCH_1_4;
    const unsigned char *d;
    unsigned char *p;
    unsigned char *l1pre = l1_temp;
    L1Pre *l1preinit = &L1_Signalling[0].l1pre_data;
    int g, o, index;

    temp = l1preinit->type;
    for (int n = 7; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    l1pre[offset_bits++] = l1preinit->bwt_ext;
    temp = l1preinit->s1;
    for (int n = 2; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->s2;
    for (int n = 2; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    if (fef_present == FALSE)
    {
        l1pre[offset_bits++] = 0;
    }
    else
    {
        l1pre[offset_bits++] = 1;
    }
    l1pre[offset_bits++] = l1preinit->l1_repetition_flag;
    temp = l1preinit->guard_interval;
    for (int n = 2; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->papr;
    for (int n = 3; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->l1_mod;
    for (int n = 3; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->l1_cod;
    for (int n = 1; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->l1_fec_type;
    for (int n = 1; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->l1_post_size;
    for (int n = 17; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->l1_post_info_size;
    for (int n = 17; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->pilot_pattern;
    for (int n = 3; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->tx_id_availability;
    for (int n = 7; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->cell_id;
    for (int n = 15; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->network_id;
    for (int n = 15; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->t2_system_id;
    for (int n = 15; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->num_t2_frames;
    for (int n = 7; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->num_data_symbols;
    for (int n = 11; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->regen_flag;
    for (int n = 2; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    l1pre[offset_bits++] = l1preinit->l1_post_extension;
    temp = l1preinit->num_rf;
    for (int n = 2; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->current_rf_index;
    for (int n = 2; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1preinit->t2_version;
    for (int n = 3; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    l1pre[offset_bits++] = l1preinit->l1_post_scrambled;
    l1pre[offset_bits++] = l1preinit->t2_base_lite;
    temp = l1preinit->reserved;
    for (int n = 3; n >= 0; n--)
    {
        l1pre[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    offset_bits += add_crc32_bits(l1pre, offset_bits);
    /* Padding */
    for (int n = KBCH_1_4 - offset_bits - 1; n >= 0; n--)
    {
        l1pre[offset_bits++] = 0;
    }
    /* BCH */
    offset_bits = 0;
    memset(shift, 0, sizeof(unsigned int) * 6);
    for (int j = 0; j < KBCH_1_4; j++)
    {
        value = l1pre[offset_bits++];
        b = (value ^ ((shift[5] & 0x01000000) ? 1 : 0));
        reg_6_shift(shift);
        if (b)
        {
            shift[0] ^= m_poly_s_12[0];
            shift[1] ^= m_poly_s_12[1];
            shift[2] ^= m_poly_s_12[2];
            shift[3] ^= m_poly_s_12[3];
            shift[4] ^= m_poly_s_12[4];
            shift[5] ^= m_poly_s_12[5];
        }
    }
    for (int n = 0; n < NBCH_PARITY; n++)
    {
        l1pre[offset_bits++] = (shift[5] & 0x01000000) ? 1 : 0;
        reg_6_shift(shift);
    }
    /* LDPC */
    d = l1_temp;
    p = &l1_temp[NBCH_1_4];
    memset(p, 0, sizeof(unsigned char)*plen);
    for(int j = 0; j < l1pre_ldpc_encode.table_length; j++)
    {
        p[l1pre_ldpc_encode.p[j]] ^= d[l1pre_ldpc_encode.d[j]];
    }
    for(int j = 1; j < plen; j++)
    {
       p[j] ^= p[j-1];
    }
    /* Puncturing */
    for (int c = 0; c < 31; c++)
    {
        g = pre_puncture[c];
        for (int c2 = 0; c2 < 360; c2++)
        {
            o = (c2 * 36) + g + NBCH_1_4;
            l1_temp[o] = 0x55;
        }
    }
    g = pre_puncture[31];
    for (int c2 = 0; c2 < 328; c2++)
    {
        o = (c2 * 36) + g + NBCH_1_4;
        l1_temp[o] = 0x55;
    }
    /* remove padding and punctured bits, BPSK modulate */
    index = 0;
    for (int w = 0; w < KSIG_PRE; w++)
    {
        out[index++] = m_bpsk[l1_temp[w]];
    }
    for (int w = 0; w < NBCH_PARITY; w++)
    {
        out[index++] = m_bpsk[l1_temp[w + KBCH_1_4]];
    }
    for (int w = 0; w < FRAME_SIZE_SHORT - NBCH_1_4; w++)
    {
        if (l1_temp[w + NBCH_1_4] != 0x55)
        {
            out[index++] = m_bpsk[l1_temp[w + NBCH_1_4]];
        }
    }
}

void framemapper_cc_impl::add_l1post(gr_complex *out, int t2_frame_num)
{
    int temp, offset_bits = 0;
    unsigned char b, value;
    unsigned int shift[6];
    int plen = FRAME_SIZE_SHORT - NBCH_1_4;
    const unsigned char *d;
    unsigned char *p;
    unsigned char *l1post = l1_interleave;
    L1Post *l1postinit = &L1_Signalling[0].l1post_data;
    int m, g, o, last, index;
    const int *post_padding;
    const int *post_puncture;
    int rows, numCols, mod, offset, pack, produced;
    unsigned char *cols[12];

    temp = l1postinit->sub_slices_per_frame;
    for (int n = 14; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->num_plp;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->num_aux;
    for (int n = 3; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->aux_config_rfu;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->rf_idx;
    for (int n = 2; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->frequency;
    for (int n = 31; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    if (fef_present == TRUE)
    {
        temp = 0;
        for (int n = 3; n >= 0; n--)
        {
            l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
        }
        temp = fef_length;
        for (int n = 21; n >= 0; n--)
        {
            l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
        }
        temp = fef_interval;
        for (int n = 7; n >= 0; n--)
        {
            l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
        }
    }
    temp = l1postinit->plp_id;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->plp_type;
    for (int n = 2; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->plp_payload_type;
    for (int n = 4; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    l1post[offset_bits++] = l1postinit->ff_flag;
    temp = l1postinit->first_rf_idx;
    for (int n = 2; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->first_frame_idx;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->plp_group_id;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->plp_cod;
    for (int n = 2; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->plp_mod;
    for (int n = 2; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    l1post[offset_bits++] = l1postinit->plp_rotation;
    temp = l1postinit->plp_fec_type;
    for (int n = 1; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->plp_num_blocks_max;
    for (int n = 9; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->frame_interval;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->time_il_length;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    l1post[offset_bits++] = l1postinit->time_il_type;
    l1post[offset_bits++] = l1postinit->in_band_a_flag;
    l1post[offset_bits++] = l1postinit->in_band_b_flag;
    temp = l1postinit->reserved_1;
    for (int n = 10; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->plp_mode;
    for (int n = 1; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    l1post[offset_bits++] = l1postinit->static_flag;
    l1post[offset_bits++] = l1postinit->static_padding_flag;
    temp = l1postinit->fef_length_msb;
    for (int n = 1; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->reserved_2;
    for (int n = 29; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = t2_frame_num;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->sub_slice_interval;
    for (int n = 21; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->type_2_start;
    for (int n = 21; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->l1_change_counter;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->start_rf_idx;
    for (int n = 2; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->reserved_3;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->plp_id_dynamic;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->plp_start;
    for (int n = 21; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->plp_num_blocks;
    for (int n = 9; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->reserved_4;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    temp = l1postinit->reserved_5;
    for (int n = 7; n >= 0; n--)
    {
        l1post[offset_bits++] = temp & (1 << n) ? 1 : 0;
    }
    offset_bits += add_crc32_bits(l1post, offset_bits);
    if (l1_scrambled == TRUE)
    {
        for (int n = 0; n < offset_bits; n++)
        {
            l1post[n] = l1post[n] ^ l1_randomize[n];
        }
    }
    /* Padding */
    switch (l1_constellation)
    {
        case gr::dvbt2::L1_MOD_BPSK:
            post_padding = post_padding_bqpsk;
            break;
        case gr::dvbt2::L1_MOD_QPSK:
            post_padding = post_padding_bqpsk;
            break;
        case gr::dvbt2::L1_MOD_16QAM:
            post_padding = post_padding_16qam;
            break;
        case gr::dvbt2::L1_MOD_64QAM:
            post_padding = post_padding_64qam;
            break;
        default:
            post_padding = post_padding_bqpsk;
            break;
    }
    memset(l1_map, 0, KBCH_1_2);
    if (offset_bits <= 360)
    {
        m = 20 - 1;
        last = 360 - offset_bits;
    }
    else
    {
        m = (KBCH_1_2 - offset_bits) / 360;
        last = KBCH_1_2 - offset_bits - (360 * m);
    }
    for (int n = 0; n < m; n++)
    {
        index = post_padding[n] * 360;
        if (post_padding[n] == 19)
        {
            for (int w = 0; w < 192; w++)
            {
                l1_map[index++] = 0x7;
            }
        }
        else
        {
            for (int w = 0; w < 360; w++)
            {
                l1_map[index++] = 0x7;
            }
        }
    }
    if (post_padding[m] == 19)
    {
        index = (post_padding[m] * 360) + 192 - last;
    }
    else
    {
        index = (post_padding[m] * 360) + 360 - last;
    }
    for (int w = 0; w < last; w++)
    {
        l1_map[index++] = 0x7;
    }
    index = 0;
    l1post = l1_temp;
    for (int n = 0; n < KBCH_1_2; n++)
    {
        if (l1_map[n] != 0x7)
        {
            l1post[n] = l1_interleave[index++];
        }
        else
        {
            l1post[n] = 0;
        }
    }
    /* BCH */
    offset_bits = 0;
    memset(shift, 0, sizeof(unsigned int) * 6);
    for (int j = 0; j < KBCH_1_2; j++)
    {
        value = l1post[offset_bits++];
        b = (value ^ ((shift[5] & 0x01000000) ? 1 : 0));
        reg_6_shift(shift);
        if (b)
        {
            shift[0] ^= m_poly_s_12[0];
            shift[1] ^= m_poly_s_12[1];
            shift[2] ^= m_poly_s_12[2];
            shift[3] ^= m_poly_s_12[3];
            shift[4] ^= m_poly_s_12[4];
            shift[5] ^= m_poly_s_12[5];
        }
    }
    for (int n = 0; n < NBCH_PARITY; n++)
    {
        l1post[offset_bits++] = (shift[5] & 0x01000000) ? 1 : 0;
        reg_6_shift(shift);
    }
    /* LDPC */
    d = l1_temp;
    p = &l1_temp[NBCH_1_2];
    memset(p, 0, sizeof(unsigned char)*plen);
    for(int j = 0; j < l1post_ldpc_encode.table_length; j++)
    {
        p[l1post_ldpc_encode.p[j]] ^= d[l1post_ldpc_encode.d[j]];
    }
    for(int j = 1; j < plen; j++)
    {
       p[j] ^= p[j-1];
    }
    /* Puncturing */
    switch (l1_constellation)
    {
        case gr::dvbt2::L1_MOD_BPSK:
            post_puncture = post_puncture_bqpsk;
            break;
        case gr::dvbt2::L1_MOD_QPSK:
            post_puncture = post_puncture_bqpsk;
            break;
        case gr::dvbt2::L1_MOD_16QAM:
            post_puncture = post_puncture_16qam;
            break;
        case gr::dvbt2::L1_MOD_64QAM:
            post_puncture = post_puncture_64qam;
            break;
        default:
            post_puncture = post_puncture_bqpsk;
            break;
    }
    for (int c = 0; c < (N_punc / 360); c++)
    {
        g = post_puncture[c];
        for (int c2 = 0; c2 < 360; c2++)
        {
            o = (c2 * 25) + g + NBCH_1_2;
            l1_temp[o] = 0x55;
        }
    }
    g = post_puncture[(N_punc / 360)];
    for (int c2 = 0; c2 < (N_punc - ((N_punc / 360) * 360)); c2++)
    {
        o = (c2 * 25) + g + NBCH_1_2;
        l1_temp[o] = 0x55;
    }
    /* remove padding and punctured bits */
    index = 0;
    for (int w = 0; w < KBCH_1_2; w++)
    {
        if (l1_map[w] != 0x7)
        {
            l1_interleave[index++] = l1_temp[w];
        }
    }
    for (int w = 0; w < NBCH_PARITY; w++)
    {
        l1_interleave[index++] = l1_temp[w + KBCH_1_2];
    }
    for (int w = 0; w < FRAME_SIZE_SHORT - NBCH_1_2; w++)
    {
        if (l1_temp[w + NBCH_1_2] != 0x55)
        {
            l1_interleave[index++] = l1_temp[w + NBCH_1_2];
        }
    }
    /* Bit interleave for 16QAM and 64QAM */
    if (l1_constellation == gr::dvbt2::L1_MOD_16QAM || l1_constellation == gr::dvbt2::L1_MOD_64QAM)
    {
        if (l1_constellation == gr::dvbt2::L1_MOD_16QAM)
        {
            numCols = 8;
            rows = N_post / 8;
        }
        else
        {
            numCols = 12;
            rows = N_post / 12;
        }
        for (int j = 0; j < numCols; j++)
        {
            cols[j] = &l1_interleave[rows * j];
        }
        index = 0;
        for (int k = 0; k < rows; k++)
        {
            for (int w = 0; w < numCols; w++)
            {
                *l1post++ = *(cols[w] + index);
            }
            index++;
        }
    }
    switch (l1_constellation)
    {
        case gr::dvbt2::L1_MOD_BPSK:
            index = 0;
            produced = 0;
            for (int d = 0; d < N_post; d++)
            {
                out[produced++] = m_bpsk[l1_interleave[index++]];
            }
            break;
        case gr::dvbt2::L1_MOD_QPSK:
            mod = 2;
            index = 0;
            produced = 0;
            for (int d = 0; d < N_post / mod; d++)
            {
                pack = 0;
                for (int e = 0; e < mod; e++)
                {
                    pack |= l1_interleave[index++];
                    pack <<= 1;
                }
                pack >>= 1;
                out[produced++] = m_qpsk[pack];
            }
            break;
        case gr::dvbt2::L1_MOD_16QAM:
            mod = 4;
            index = 0;
            produced = 0;
            for (int d = 0; d < N_post / (mod * 2); d++)
            {
                pack = 0;
                for (int e = 0; e < (mod * 2); e++)
                {
                    offset = mux16[e];
                    pack |= l1_temp[index + offset];
                    pack <<= 1;
                }
                pack >>= 1;
                out[produced++] = m_16qam[pack >> 4];
                out[produced++] = m_16qam[pack & 0xf];
                index += (mod * 2);
            }
            break;
        case gr::dvbt2::L1_MOD_64QAM:
            mod = 6;
            index = 0;
            produced = 0;
            for (int d = 0; d < N_post / (mod * 2); d++)
            {
                pack = 0;
                for (int e = 0; e < (mod * 2); e++)
                {
                    offset = mux64[e];
                    pack |= l1_temp[index + offset];
                    pack <<= 1;
                }
                pack >>= 1;
                out[produced++] = m_64qam[pack >> 6];
                out[produced++] = m_64qam[pack & 0x3f];
                index += (mod * 2);
            }
            break;
    }
}

void framemapper_cc_impl::init_dummy_randomizer(void)
{
    int sr = 0x4A80;
    for (int i = 0; i < mapped_items - stream_items - 1840 - (N_post / eta_mod) - (N_FC - C_FC); i++)
    {
        int b = ((sr) ^ (sr >> 1)) & 1;
        if (b)
            dummy_randomize[i].real() = -1.0;
        else
            dummy_randomize[i].real() = 1.0;
        dummy_randomize[i].imag() = 0;
        sr >>= 1;
        if(b) sr |= 0x4000;
    }
}

void framemapper_cc_impl::init_l1_randomizer(void)
{
    int sr = 0x4A80;
    for (int i = 0; i < KBCH_1_2; i++)
    {
        int b = ((sr) ^ (sr >> 1)) & 1;
        l1_randomize[i] = b;
        sr >>= 1;
        if(b) sr |= 0x4000;
    }
}

    int
    framemapper_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];
        int index = 0;
        int read, save, count = 0;
        gr_complex *interleave = zigzag_interleave;

        for (int i = 0; i < noutput_items; i += mapped_items)
        {
            if (N_P2 == 1)
            {
                for (int j = 0; j < 1840; j++)
                {
                    *out++ = l1pre_cache[index++];
                }
                add_l1post(out, t2_frame_num);
                t2_frame_num = (t2_frame_num + 1) % t2_frames;
                out += N_post / eta_mod;
                for (int j = 0; j < stream_items; j++)
                {
                    *out++ = *in++;
                }
                index = 0;
                for (int j = 0; j < mapped_items - stream_items - 1840 - (N_post / eta_mod) - (N_FC - C_FC); j++)
                {
                    *out++ = dummy_randomize[index++];
                }
                for (int j = 0; j < N_FC - C_FC; j++)
                {
                    *out++ = unmodulated[0];
                }
            }
            else
            {
                for (int j = 0; j < 1840; j++)
                {
                    *interleave++ = l1pre_cache[index++];
                }
                add_l1post(interleave, t2_frame_num);
                t2_frame_num = (t2_frame_num + 1) % t2_frames;
                interleave += N_post / eta_mod;
                for (int j = 0; j < stream_items; j++)
                {
                    *interleave++ = *in++;
                }
                index = 0;
                for (int j = 0; j < mapped_items - stream_items - 1840 - (N_post / eta_mod) - (N_FC - C_FC); j++)
                {
                    *interleave++ = dummy_randomize[index++];
                }
                for (int j = 0; j < N_FC - C_FC; j++)
                {
                    *interleave++ = unmodulated[0];
                }
                interleave = zigzag_interleave;
                read = 0;
                index = 0;
                for (int n = 0; n < N_P2; n++)
                {
                    save = read;
                    for (int j = 0; j < 1840 / N_P2; j++)
                    {
                        out[index++] = interleave[read];
                        count++;
                        read += N_P2;
                    }
                    read = save + 1;
                    index += C_P2 - (1840 / N_P2);
                }
                read = 1840;
                index = 1840 / N_P2;
                for (int n = 0; n < N_P2; n++)
                {
                    save = read;
                    for (int j = 0; j < (N_post / eta_mod) / N_P2; j++)
                    {
                        out[index++] = interleave[read];
                        count++;
                        read += N_P2;
                    }
                    read = save + 1;
                    index += C_P2 - ((N_post / eta_mod) / N_P2);
                }
                read = 1840 + (N_post / eta_mod);
                index = (1840 / N_P2) + ((N_post / eta_mod) / N_P2);
                for (int n = 0; n < N_P2; n++)
                {
                    for (int j = 0; j < C_P2 - (1840 / N_P2) - ((N_post / eta_mod) / N_P2); j++)
                    {
                        out[index++] = interleave[read++];
                        count++;
                    }
                    index += C_P2 - (C_P2 - (1840 / N_P2) - ((N_post / eta_mod) / N_P2));
                }
                index -= C_P2 - (C_P2 - (1840 / N_P2) - ((N_post / eta_mod) / N_P2));
                for (int j = 0; j < mapped_items - count; j++)
                {
                    out[index++] = interleave[read++];
                }
                out += mapped_items;
            }
        }

        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (stream_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

const int framemapper_cc_impl::ldpc_tab_1_4S[9][13]=
{
    {12,6295,9626,304,7695,4839,4936,1660,144,11203,5567,6347,12557},
    {12,10691,4988,3859,3734,3071,3494,7687,10313,5964,8069,8296,11090},
    {12,10774,3613,5208,11177,7676,3549,8746,6583,7239,12265,2674,4292},
    {12,11869,3708,5981,8718,4908,10650,6805,3334,2627,10461,9285,11120},
    {3,7844,3079,10773,0,0,0,0,0,0,0,0,0},
    {3,3385,10854,5747,0,0,0,0,0,0,0,0,0},
    {3,1360,12010,12202,0,0,0,0,0,0,0,0,0},
    {3,6189,4241,2343,0,0,0,0,0,0,0,0,0},
    {3,9840,12726,4977,0,0,0,0,0,0,0,0,0}
};

const int framemapper_cc_impl::ldpc_tab_1_2S[20][9]=
{
    {8,20,712,2386,6354,4061,1062,5045,5158},
    {8,21,2543,5748,4822,2348,3089,6328,5876},
    {8,22,926,5701,269,3693,2438,3190,3507},
    {8,23,2802,4520,3577,5324,1091,4667,4449},
    {8,24,5140,2003,1263,4742,6497,1185,6202},
    {3,0,4046,6934,0,0,0,0,0},
    {3,1,2855,66,0,0,0,0,0},
    {3,2,6694,212,0,0,0,0,0},
    {3,3,3439,1158,0,0,0,0,0},
    {3,4,3850,4422,0,0,0,0,0},
    {3,5,5924,290,0,0,0,0,0},
    {3,6,1467,4049,0,0,0,0,0},
    {3,7,7820,2242,0,0,0,0,0},
    {3,8,4606,3080,0,0,0,0,0},
    {3,9,4633,7877,0,0,0,0,0},
    {3,10,3884,6868,0,0,0,0,0},
    {3,11,8935,4996,0,0,0,0,0},
    {3,12,3028,764,0,0,0,0,0},
    {3,13,5988,1057,0,0,0,0,0},
    {3,14,7411,3450,0,0,0,0,0}
};

    const int framemapper_cc_impl::pre_puncture[36] = 
    {
        27, 13, 29, 32, 5, 0, 11, 21, 33, 20, 25, 28, 18, 35, 8, 3, 9, 31, 22, 24, 7, 14, 17, 4, 2, 26, 16, 34, 19, 10, 12, 23, 1, 6, 30, 15
    };

    const int framemapper_cc_impl::post_padding_bqpsk[20] = 
    {
        18, 17, 16, 15, 14, 13, 12, 11, 4, 10, 9, 8, 3, 2, 7, 6, 5, 1, 19, 0
    };

    const int framemapper_cc_impl::post_padding_16qam[20] = 
    {
        18, 17, 16, 15, 14, 13, 12, 11, 4, 10, 9, 8, 7, 3, 2, 1, 6, 5, 19, 0
    };

    const int framemapper_cc_impl::post_padding_64qam[20] = 
    {
        18, 17, 16, 4, 15, 14, 13, 12, 3, 11, 10, 9, 2, 8, 7, 1, 6, 5, 19, 0
    };

    const int framemapper_cc_impl::post_puncture_bqpsk[25] = 
    {
        6, 4, 18, 9, 13, 8, 15, 20, 5, 17, 2, 24, 10, 22, 12, 3, 16, 23, 1, 14, 0, 21, 19, 7, 11
    };

    const int framemapper_cc_impl::post_puncture_16qam[25] = 
    {
        6, 4, 13, 9, 18, 8, 15, 20, 5, 17, 2, 22, 24, 7, 12, 1, 16, 23, 14, 0, 21, 10, 19, 11, 3
    };

    const int framemapper_cc_impl::post_puncture_64qam[25] = 
    {
        6, 15, 13, 10, 3, 17, 21, 8, 5, 19, 2, 23, 16, 24, 7, 18, 1, 12, 20, 0, 4, 14, 9, 11, 22
    };

    const int framemapper_cc_impl::mux16[8] =
    {
        7, 1, 3, 5, 2, 4, 6, 0
    };

    const int framemapper_cc_impl::mux64[12] =
    {
        11, 8, 5, 2, 10, 7, 4, 1, 9, 6, 3, 0
    };

  } /* namespace dvbt2 */
} /* namespace gr */

