/*
 *  COPYRIGHT (C) 2000-2021 ZUUT, INC. ALL RIGHTS RESERVED.
 */
#ifndef BIZOBJ_CREDITCARD
#define BIZOBJ_CREDITCARD

#line 14 "test_app/test_app.moc" 




   static char header_code= "   typically this is code inserted at the beginning of a header file.   Comments are removed.";



#line 17 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/creditcard/CreditCard.h" 

#include <memory>
#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
/**
 * <P>
 * CreditCard
 * a CreditCard.
 *   A credit card number.;
 * </P>
 *
 * @version $Id$
 */
class CreditCard : public BizObject  {
public:
private:    
    /**The creditCardNumber property - the credit card number. */
    private String creditCardNumber;
    /**The creditCardType property - the type of credit card. */
    private CreditCardType creditCardType;
    /**The creditCardExpDate property - the credit card expiration date. */
    private String creditCardExpDate;
};

#line 127 "test_app/test_app.moc" 


   static char header_end_code= "   typically this is code inserted at the end of a header file.";


#line 50 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/creditcard/CreditCard.h" 

#endif // BIZOBJ_CREDITCARD
