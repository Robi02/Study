package com.rblee.toy.controller;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class MainController {
    @GetMapping("/index")
    public String index() {
        return "Hello Spring Boot!";
    }
}