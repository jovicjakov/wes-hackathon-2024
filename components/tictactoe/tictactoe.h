#define TICTACTOE_QUEUE_SIZE (20U)

#define MAX_SYMBOLS_ON_FIELD 9

typedef enum
{
   DEVICE,
   SERVER
} tictactoe_turn_t;

typedef struct
{
   int index_of_X[MAX_SYMBOLS_ON_FIELD];
   int index_of_O[MAX_SYMBOLS_ON_FIELD];
   tictactoe_turn_t turn;
} tictactoe_handler_t;

esp_err_t tictactoe_init(void);