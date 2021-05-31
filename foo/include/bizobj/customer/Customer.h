/*
 *  COPYRIGHT (C) 2000-2021 ZUUT, INC. ALL RIGHTS RESERVED.
 */
#ifndef BIZOBJ_CUSTOMER
#define BIZOBJ_CUSTOMER

#line 14 "test_app/test_app.moc" 




   static char header_code= "   typically this is code inserted at the beginning of a header file.   Comments are removed.";



#line 17 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/customer/Customer.h" 

#include <memory>
#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
/**
 * <P>
 * Customer
 * a Customer.
 *   A customer's account;
 * </P>
 *
 * @version $Id$
 */
class Customer : public BizObject  {
public:
private:    
    /**The firstName property - the first name of the customer. */
    private String firstName;
    /**The lastName property - the last name. */
    private String lastName;
    /**The telephone property - the telephone. */
    private String telephone;
    /**The email property - the email address. */
    private String email;
    /**The login property - the login. */
    private String login;
    /**The password property - the password. */
    private String password;
    /**The uuid property - a UUID for the customer. */
    private String uuid;
    /**The dateOfBirth property - The date of birth. */
    private Date dateOfBirth;
    /**The age property - The age of the customer. */
    private Integer age;
    /**The homeAddressId property - The home address. */
    private Long homeAddressId;
};

#line 127 "test_app/test_app.moc" 


   static char header_end_code= "   typically this is code inserted at the end of a header file.";


#line 64 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/customer/Customer.h" 

#endif // BIZOBJ_CUSTOMER
