package com.rblee.toy.controller;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.servlet.ModelAndView;

@RestController
public class UsersController {

    Logger logger = LoggerFactory.getLogger(UsersController.class);

    @GetMapping("/users/register")
    public ModelAndView register() {
        logger.info("register called!");
        return new ModelAndView("register");
    }
}