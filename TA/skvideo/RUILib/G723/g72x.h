#ifndef _G72X_H
#define	_G72X_H

#define	AUDIO_ENCODING_LINEAR	(3)	/* PCM 2's-complement (0-center) */

/*
 * The following is the definition of the state structure
 * used by the G.721/G.723 encoder and decoder to preserve their internal
 * state between successive calls.  The meanings of the majority
 * of the state structure fields are explained in detail in the
 * CCITT Recommendation G.721.  The field names are essentially indentical
 * to variable names in the bit level description of the coding algorithm
 * included in this Recommendation.
 */
struct g72x_state
{
    long yl; 	/* Locked or steady state step size multiplier. */
    short yu; 	/* Unlocked or non-steady state step size multiplier. */
    short dms; 	/* Short term energy estimate. */
    short dml; 	/* Long term energy estimate. */
    short ap; 	/* Linear weighting coefficient of 'yl' and 'yu'. */

    short a[2]; 	/* Coefficients of pole portion of prediction filter. */
    short b[6]; 	/* Coefficients of zero portion of prediction filter. */
    short pk[2]; 	/*
    			 * Signs of previous two samples of a partially
    			 * reconstructed signal.
    			 */
    short dq[6]; 	/*
    			 * Previous 6 samples of the quantized difference
    			 * signal represented in an internal floating point
    			 * format.
    			 */
    short sr[2]; 	/*
    			 * Previous 2 samples of the quantized difference
    			 * signal represented in an internal floating point
    			 * format.
    			 */
    char td; 	/* delayed tone detect, new in 1988 version */
};

/* External function definitions. */

extern void g72x_init_stat(struct g72x_state *);
extern int g721_encoder(int sample, int in_coding, struct g72x_state *state_ptr);
extern int g721_decoder(int code, int out_coding, struct g72x_state *state_ptr);
extern int g723_24_encoder(int sample, int in_coding, struct g72x_state *state_ptr);
extern int g723_24_decoder(int code, int out_coding, struct g72x_state *state_ptr);
extern int g723_40_encoder( int sample, int in_coding, struct g72x_state *state_ptr);
extern int g723_40_decoder(int code, int out_coding, struct g72x_state *state_ptr);

int predictor_zero( struct g72x_state *state_ptr );
int predictor_pole( struct g72x_state *state_ptr );
int step_size( struct g72x_state *state_ptr );
int quantize( int d, int y, short *table, int size );
int reconstruct( int sign, int dqln, int y );
void update( int code_size, int y, int wi, int fi, int dq, int sr, int dqsez, struct g72x_state *state_ptr );

void g72x_init_state( struct g72x_state *state_ptr);

#endif /* !_G72X_H */
