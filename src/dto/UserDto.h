#ifndef USER_DTO_H
#define USER_DTO_H

#include <string>
#include "../model/User.h"

struct UserDto {
    std::string publicUserId;
    std::string displayUsername;
    std::string contactInfo;
    std::string status;
    std::string roleName;
    
    static UserDto fromEntity(const User& user) {
        UserDto dto;
        dto.publicUserId = "U_" + std::to_string(user.userId);
        dto.displayUsername = user.username;
        dto.contactInfo = user.email + " / " + user.phone;
        dto.status = user.isActive ? "ACTIVE" : "INACTIVE";
        
        if (user.role == 1) {
            dto.roleName = "ADMIN";
        } else if (user.role == 2) {
            dto.roleName = "MANAGER";
        } else {
            dto.roleName = "USER";
        }
        
        return dto;
    }
};

#endif // USER_DTO_H
