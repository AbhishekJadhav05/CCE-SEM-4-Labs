#!/bin/bash
read -p "Enter Basic Salary: " basic
read -p "Enter TA: " ta
bonus=$(echo "scale=2; $basic*0.10" | bc)
gross_salary=$(echo "scale=2; $basic+$ta+$bonus" | bc)
printf "Gross Salary: %.2f\n" "$gross_salary"`
