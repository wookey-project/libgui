#ifndef MYPIN_H
#define MYPIN_H

/*
 * request a text content to the user (blocking call while the user
 * is writing)
 */
void pin_request_string(const char *title,
                        uint32_t    title_len,
                        int x1, int x2, int y1, int y2,
                        char *string, uint8_t maxlen);

/*
 * request a numerical content to the user (blocking call while the
 * user is writing)
 */
uint8_t pin_request_digits(const char *title,
                           uint32_t    title_len,
                           int x1,int x2, int y1, int y2,
                           char *mypin, uint8_t max_pin_len);

/*
 * request a validation of a string (the string is shown, the user
 * has to say 'OK' or 'KO'.
 * The validation menu title is using title arg to complete the
 * 'please confirm' string
 */
uint8_t pin_request_string_validation(const char *title,
                                      const char *string,
                                      uint8_t string_len);

#endif
